#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Load and render a 3D scene in Mistuba3, using a JSON file as input.

Prerequisites:
  pip install mitsuba
  pip install numpy         # apt install python3-numpy
  pip install matplotlib    # apt install python3-matplotlib


current JSON structure: see ref/scene_v0.1c.json
"""

import os
import time
from   math import cos, sin
import json
import argparse
import matplotlib.pyplot as plt
import numpy as np
import mitsuba as mi
import drjit
import gc
#import resource


##########################################################

textures_cache = {}

##########################################################

def fprint(printable):
    """
    Print and flush output.

    Arguments:
    - printable: the printable object to print (printable Python object).
    """
    #pprint.pprint(printable, indent=2)
    #sys.stdout.flush()
    print(printable, flush=True)

##########################################################

def show_image(img):
    """
    Display one image using matplotlib.
    """
    plt.figure(figsize=(img.shape[1],img.shape[0]), dpi=1, layout='tight')
    ### new figure at each call, full scale, no border
    plt.axis("off")
    plt.imshow(img ** (1.0 / 2.2))  # approximate sRGB tonemapping
    #plt.show()

##########################################################

def seconds_since(start):
    """
    Return elapsed time since <start_time_ns>.

    Argument:
    - start: starting time in nanoseconds as returned by time.time_ns()

    Return elapsed time in seconds with millisecond precision
    """
    now = time.time_ns()  # in nanoseconds
    return int((now - start)/1e6)/1e3  # in s with ms precision

##########################################################

def find_texture_filepath(obj_filepath):
    """
    Find the texture image file and texture mask used by an OBJ file.

    Argument:
    - obj_filepath: path to the OBJ file (str)

    Return the path to the texture image file and the path to the texture mask
    (couple of strings).
    """
    # find mtl filename in obj file
    mtl_filename = None
    with open(obj_filepath, encoding='utf-8') as file:
        for line in file:
            if line.startswith('mtllib'):
                mtl_filename = line.split()[1]
                break
    #DBG print('mtl_filename=', mtl_filename)
    if not mtl_filename:
        print(f'Warning: no mtl file referenced in {obj_filepath}.')
        return (None, None)

    # find texture image filename in mtl file
    tex_filename  = None
    mask_filename = None
    obj_filedir  = os.path.dirname(obj_filepath)
    mtl_filepath = os.path.join(obj_filedir, mtl_filename)
    try:
        with open(mtl_filepath, encoding='utf-8') as file:
            for line in file:
                if line.startswith('map_Kd'):
                    tex_filename = line.split()[1]
                elif line.startswith('map_d'):
                    mask_filename = line.split()[1]
                if tex_filename and mask_filename:
                    break
    except FileNotFoundError:
        print(f'Warning: mtl file "{mtl_filepath}" not found.')
        return (None, None)

    #DBG print(f'tex_filename={tex_filename}  mask_filename={mask_filename}')
    assert tex_filename

    tex_filepath  = os.path.join(obj_filedir, tex_filename)
    mask_filepath = os.path.join(obj_filedir, mask_filename) if mask_filename\
                    else None

    return (tex_filepath, mask_filepath)

##########################################################

def euler_to_mat(theta) :
    """
    Calculate the rotation matrix corresponding to the given euler angles.
    Ref: https://learnopencv.com/rotation-matrix-to-euler-angles/
         https://fr.wikipedia.org/wiki/Matrice_de_rotation

    Arguments:
    - theta: list of 3 rotation angles in radians, respectively over X, Y and
             Z, from global axis to local axis.

    Return a 4x4 transformation matrix (numpy array).
    """
    # angles are already in radians, no conversion needed
    thx, thy, thz = theta

    # 1st rotation over X, new base in ancient base
    rotx = np.array([[1,          0,          0],
                     [0,   cos(thx),  -sin(thx)],
                     [0,   sin(thx),   cos(thx)]])

    # 2nd rotation over Y, new base in ancient base
    roty = np.array([[ cos(thy),   0,   sin(thy)],
                     [       0,    1,          0],
                     [-sin(thy),   0,   cos(thy)]])

    # 3rd rotation over Z, new base in ancient base
    rotz = np.array([[cos(thz),  -sin(thz),   0],
                     [sin(thz),   cos(thz),   0],
                     [       0,          0,   1]])

    # transformation from local coords to global coords
    rot33 = rotx @ roty @ rotz

    rot44 = np.array([[0.0, 0.0, 0.0, 0.0],
                      [0.0, 0.0, 0.0, 0.0],
                      [0.0, 0.0, 0.0, 0.0],
                      [0.0, 0.0, 0.0, 1.0]])
    rot44[0:3,0:3] = rot33
    #DBG print('rot44=')
    #DBG print(rot44)

    return rot44

##########################################################

def compute_transfo(translate, rotate, scale):
    """
    Compute the 4x4 transformation matrix corresponding to
    a 3x1 translation, a 3x1 scaling and a 3x1 rotation.
    """
    rotate = euler_to_mat(rotate)
    return mi.ScalarTransform4f.translate(translate) \
               @ mi.ScalarTransform4f(rotate) \
               @ mi.ScalarTransform4f.scale(scale)

##########################################################

def fix_files_path(json_dict, json_path):
    """
    Look for files paths in JSON data and fix them according to the directory
    of the JSON file.

    Arguments:
    - json_dict: content of a JSON file (dict)
    - json_path: path to the JSON file (str)
    """

    def fix_path(file_path, dir_path):
        """
        Fix relative path.
        """
        if os.path.isabs(file_path):
            # absolute path do not need a fix
            return file_path
        return os.path.normpath(os.path.join(dir_path, file_path))

    def fix_texture_paths(texture, dir_path):
        """
        Fix all paths in the texture.
        """
        for key in ('ColorMap', 'BumpMap', 'AlphaMap', 'NormalMap',
                    'OpacityMap'):
            if key in texture:
                texture[key] = fix_path(texture[key], dir_path)


    dir_path = os.path.dirname(json_path)

    if 'Objects' in json_dict:
        for obj in json_dict['Objects']:
            for mesh in obj['Definition']:
                mesh['File'] = fix_path(mesh['File'], dir_path)
                if 'Material' in mesh:
                    fix_texture_paths(mesh['Material'], dir_path)
                    if 'Layers' in mesh['Material']:
                        for layer in mesh['Material']['Layers']:
                            fix_texture_paths(layer, dir_path)

    if 'Lights' in json_dict:
        for light in json_dict['Lights']:
            if light['Type'] == 'Envmap':
                light['File'] = fix_path(light['File'], dir_path)

    if 'Import' in json_dict:
        json_dict["Import"] = \
            [fix_path(filename, dir_path) for filename in json_dict["Import"]]

##########################################################

def merge_imports(json_dict):
    """
    Merge imports in JSON dictionnary if any.

    Arguments:
    - json_dict: content of the JSON file (dict)
    """
    if not "Import" in json_dict:
        return  # nothing to do

    # loop over imported files
    for import_filepath in json_dict["Import"]:
        fprint(f'importing file "{import_filepath}".')
        with open(import_filepath, encoding='utf-8') as file:
            import_dict = json.load(file)
            fix_files_path(import_dict, import_filepath)

            # add imported data to main json dict
            for key, val in import_dict.items():
                if key in json_dict:
                    # append to existing key
                    json_dict[key].extend(val)
                else:
                    # create new key
                    json_dict[key] = val

##########################################################

def make_textured_bsdf(texture_fp, opacmap_fp=None, normmap_fp=None,
                       bumpmap_fp=None, bumpmap_in=None, uvscale=None):
    """
    Create a textured material bsdf dictionnary.

    Arguments:
    - texture_fp:  file path of the texture image (str),
    - opacmap_fp:  file path of the transparency map (str),
    - normmap_fp:  file path of the normal map (str).
    - bumpmap_fp:  file path of the bump map (str).
    - bumpmap_in:  bump map intensity (float).

    Return a bsdf description dictionnary for Mitsuba3 (dict).
    """

    if uvscale:
        to_uv = mi.ScalarTransform4f.scale([uvscale, uvscale, 1])
    else:
        to_uv = mi.ScalarTransform4f()  # identity

    if texture_fp:
        # base textured bsdf
        if False:
            # one-sided texture
            bsdf_dict = {
                'type': 'diffuse',
                'reflectance': {
                    'type':     'bitmap',
                    'filename': texture_fp,
                    'to_uv':    to_uv
                }
            }
        else:
            # two-sided texture
            bsdf_dict = {
                'type': 'twosided',
                'bsdf': {
                    'type': 'diffuse',
                    'reflectance': {
                        'type':     'bitmap',
                        'filename': texture_fp,
                        'to_uv':    to_uv
                    }
                }
            }

        # normal map and bumpmap are mutually exclusive
        if normmap_fp:
            # texture with normal map
            bsdf_dict = {
                'type': 'normalmap',
                'normalmap': {
                    'type':     'bitmap',
                    'filename': normmap_fp,
                    'raw':      True,
                    'to_uv':    to_uv
                },
                'bsdf': bsdf_dict
            }
        elif bumpmap_fp:
            # texture with bumpmap
            bsdf_dict = {
                'type': 'bumpmap',
                'arbitrary': {
                    'type':     'bitmap',
                    'filename': bumpmap_fp,
                    'raw':      True,
                    'to_uv':    to_uv
                },
                'bsdf':  bsdf_dict,
                'scale': bumpmap_in or 1.0
            }

        # opacity map is applied over texture and normal-map/bump-map
        if opacmap_fp:
            bsdf_dict = {
                'type': 'mask',
                'material': bsdf_dict,
                'opacity': {
                    'type':     'bitmap',
                    'filename': opacmap_fp,
                    'to_uv':    to_uv
                }
            }
    else:
        # no texture provided, use dummy color
        print(f'Warning: texture image {texture_fp} not found, use dummy color.')
        bsdf_dict = {
            'type': 'twosided',
            'bsdf': {
                'type': 'diffuse',
                'reflectance': {
                    'type': 'rgb',
                    'value': [0.8, 0.1, 0.1],
                }
            }
        }

    return bsdf_dict


##########################################################

def make_blend_bsdf(bsdf_0, bsdf_1, alphamap_fp):
    """
    Create a blended bsdf dictionnary.

    Arguments:
    - bsdf_0: the 1st bsdf (dict)
    - bsdf_1: the 2nd bsdf (dict)
    - alphamap_fp: the file path of the alpha texture for bsdf_1 (str)

    Return a description dictionnary for Mitsuba3 (dict).
    """
    bsdf_dict = {
        'type': 'blendbsdf',
        'weight': {
            'type': 'bitmap',
            'filename': alphamap_fp
        },
        'bsdf_0': bsdf_0,
        'bsdf_1': bsdf_1
    }
    return bsdf_dict

##########################################################

def load_mesh_from_file(filepath, material=None, transfo=None):
    """
    Load a mesh from a file and store it in a `mitsuba.<variant>.Mesh` Python
    objet.

    Arguments:
    - filepath: path to the mesh file (str),
    - material: the material attached to the mesh if any (dict).
    - transfo:  the 4x4 transformation matrix to apply to the mesh
                (mitsuba.ScalarTransform4f).

    Return a `mitsuba.<variant>.Mesh` object.
    """

    def make_textured_bsdf_from_material(material):
        """
        Create textured BSDF from material.
        Return a Mitsuba 3 BSDF dictionnary (dict).
        """
        # mandatory
        texture_fp  = material['ColorMap']
        # optional
        opacmap_fp  = material.get('OpacityMap')
        normmap_fp  = material.get('NormalMap')
        bumpmap_fp  = material.get('BumpMap')
        bumpmap_in  = material.get('BumpMapIntensity')
        uvscale     = material.get('UVScale')
        # create BSDF dict
        bsdf_dict   = make_textured_bsdf(texture_fp,
                                         opacmap_fp=opacmap_fp,
                                         normmap_fp=normmap_fp,
                                         bumpmap_fp=bumpmap_fp,
                                         bumpmap_in=bumpmap_in,
                                         uvscale=uvscale)
        return bsdf_dict


    file_ext = filepath.lower().split('.')[-1]
    assert file_ext in ('obj', 'serialized')

    # creat mesh description
    mesh_dict = {
        'type'        : file_ext,
        'filename'    : filepath,
        'face_normals': False,     # False = use file normals
    }

    if material:
        if material['Type'] == 'Textured':
            colormap_fp = material['ColorMap']
            if colormap_fp not in textures_cache:
                # if the texture is used for the 1st time, load it and
                # add it to cache
                print(f'NEW TEXTURE {colormap_fp}')
                bsdf_dict  = make_textured_bsdf_from_material(material)
                tex_loaded = mi.load_dict(bsdf_dict)
                textures_cache[colormap_fp] = tex_loaded
            else:
                # retrieve already loaded texture from cache
                tex_loaded = textures_cache[colormap_fp]
                print(f'TEXTURE {colormap_fp} FOUND IN CACHE')
            mesh_dict['bsdf'] = tex_loaded
        elif material['Type'] == 'Blended':
            bsdf_dict = make_textured_bsdf_from_material(material['Layers'][0])

            for layer in material['Layers'][1:]:
                layer_dict  = make_textured_bsdf_from_material(layer)
                alphamap_fp = layer['AlphaMap']
                bsdf_dict   = make_blend_bsdf(bsdf_dict, layer_dict,
                                              alphamap_fp)
            mesh_dict['bsdf'] = bsdf_dict
            #DBG fprint('bsdf_dict=\n') ; fprint(bsdf_dict)
        elif material['Type'] == 'DiffuseColor':
            mesh_dict['bsdf'] = {
                'type': 'twosided',
                'bsdf': {
                    'type': 'diffuse',
                    'reflectance': {
                        'type': 'rgb',
                        'value': material['Color'],
                    }
                }
            }
        else:
            print(f'Warning: unsupported material type {material["Type"]}.')
            # no texture
            mesh_dict['bsdf'] = {
                'type': 'twosided',
                'bsdf': {
                    'type': 'diffuse',
                    'reflectance': {
                        'type': 'rgb',
                        'value': [0.0, 0.0, 1.0],
                    }
                }
            }
    else:
        # no material provided in scene JSON
        # retrieve texture file from mesh file
        if file_ext == 'obj':
            tex_filepath, mask_filepath = find_texture_filepath(filepath)
        else:
            tex_filepath, mask_filepath = None, None
        mesh_dict['bsdf'] = make_textured_bsdf(tex_filepath,
                                               opacmap_fp=mask_filepath)

    if transfo:
        mesh_dict['to_world'] = transfo

    # load mesh
    mesh = mi.load_dict(mesh_dict)
    #DBG fprint('mesh=') ; fprint(mesh)               #DBG
    #DBG fprint('mesh.bsdf()=') ; fprint(mesh.bsdf()) #DBG

    return mesh

##########################################################

def prune_objects(json_dict):
    """
    Remove unused objects (aka non-instanciated) from JSON scene dict.

    Arguments:
    - json_dict: content of the JSON file (dict)

    Return the modified json_dict (dict).
    """

    # build the list of objects really used in the scene (aka instanciated)
    used_object_names = set()
    for obj_inst in json_dict['ObjectsInstances']:
        used_object_names.add(obj_inst['Ref'])

    # build used objects list
    used_objects = [obj for obj in json_dict['Objects'] if obj['Name'] in \
                    used_object_names]
    #DBG fprint(f'used_objects = {used_objects}') #DBG

    # replace complete-objects list by used-objects list
    json_dict['Objects'] = used_objects

##########################################################

def load_meshes(json_dict):
    """
    Load all meshes referenced by a JSON scene dict.

    Arguments:
    - json_dict: content of the JSON file (dict)

    Return a dictionnary of shapegroups.
    """

    shapegroups = {}
    load_cnt = 0

    # loop over Objects in JSON
    for obj in json_dict['Objects']:
        obj_name = obj['Name']
        shapegroup = {
            'type': 'shapegroup'
        }
        mesh_cnt = 0

        # load Object's meshes
        for mesh in obj['Definition']:
            mesh_cnt += 1
            mesh_filepath = mesh['File']
            mesh_material = mesh['Material'] if 'Material' in mesh else None
            inst_cnt = 0
            # loop over instances of mesh
            for instance in mesh['Instances']:
                inst_cnt += 1
                transfo  = compute_transfo(instance['Translate'],
                                           instance['Rotate'],
                                           instance['Scale'])
                # load one mesh with its transfo
                mesh_loaded = load_mesh_from_file(mesh_filepath, mesh_material,
                                                  transfo)
                load_cnt += 1
                # add mesh to object's shapegroup
                shapegroup[f'mesh_{mesh_cnt}_inst_{inst_cnt}'] = mesh_loaded

        # add object's shapegroup to shapegroups
        shapegroups[f'sh_grp_{obj_name}'] = shapegroup

    return shapegroups, load_cnt

##########################################################

def make_scene_dict_static(shapegroups, json_dict):
    """
    Create the scene dictionnary for Mitsuba3, taking as input the content of
    a JSON file. Make the static part of the scene.

    Arguments:
    - shapegroups: dictionnary of object's shapegroups (dict)
    - json_dict: content of the JSON file (dict)

    Return the scene dictionnary of static parts, appropriate for
    Mitsuba3 (dict), and a dict of instances, lights and cameras numbers.
    """
    scene_dict = {
        'type'      : 'scene',
        'integrator': {
            'type': 'volpath',
            'max_depth': 32
        },
    }

    # add shapegroups
    scene_dict.update(shapegroups)

    # add static objects
    inst_cnt = add_object_instances(scene_dict, json_dict, static=True)

    # add static lights
    lights_cnt = add_lights(scene_dict, json_dict, static=True)

    # add static cameras
    cameras_cnt = add_cameras(scene_dict, json_dict, static=True)

    #DBG print('scene_dict=', scene_dict)
    return scene_dict, {'instances_cnt': inst_cnt, 'lights_cnt': lights_cnt,
                        'cameras_cnt': cameras_cnt }

##########################################################

def add_cameras(scene_dict, json_dict, static, frame_id=None):
    """
    Add cameras to the scene dictionnary.

    Arguments:
    - scene_dict: the scene dictionnary (dict)
    - json_dict: content of the JSON file (dict)
    - static: static/dynamic part of the scene (True/False)
    - frame_id: id of the frame for dynamic part of the scene (int)

    Return the number of cameras created.
    """
    assert static in (True, False)

    # retrieve camera resolution from scene data
    resolution = json_dict['Scene'].get('Resolution') or (1024, 768)
    assert len(resolution) == 2

    cameras_inst_cnt = 0

    # loop over cameras
    for camera in json_dict['Cameras']:
        # retrieve the list of instances to create
        if static and 'Instances' in camera:
            # static objects
            instances_list = camera['Instances']
        elif (not static) and 'Frames' in camera:
            # dynamic objects
            instances_list = camera['Frames'][frame_id]['Instances']
        else:
            # no static or dynamic instance to build
            continue

        camera_name = camera['Name']
        fov         = camera.get('FOV') or 45
        fprint(f'Camera resolution {resolution}, FOV {fov}.')

        # loop over camera instances
        for inst_cnt, camera_instance in enumerate(instances_list, start=1):
            inst_name = f'sensor__{camera_name}_inst_{inst_cnt}'
            scene_dict[inst_name] = {
                    'type': 'perspective',
                    'fov' : fov,
                    'to_world': mi.ScalarTransform4f.look_at(
                                    origin=camera_instance['Eye'],
                                    target=camera_instance['At'],
                                    up=camera_instance['Up'],
                                ),
                    'film'    : {
                        'type': 'hdrfilm',
                        # resolution
                        'width':  resolution[0],
                        'height': resolution[1],
                    },
                }

        cameras_inst_cnt += inst_cnt

    return cameras_inst_cnt

##########################################################

def add_lights(scene_dict, json_dict, static, frame_id=None):
    """
    Add lights to the scene dictionnary.

    Arguments:
    - scene_dict: the scene dictionnary (dict)
    - json_dict: content of the JSON file (dict)
    - objects: dictionnary of objects (dict)
    - static: static/dynamic part of the scene (True/False)
    - frame_id: id of the frame for dynamic part of the scene (int)

    Return the number of lights created.
    """
    assert static in (True, False)

    lights_inst_cnt = 0

    # loop over lights
    for light in json_dict['Lights']:
        # retrieve the list of instances to create
        if static and 'Instances' in light:
            # static objects
            instances_list = light['Instances']
        elif (not static) and 'Frames' in light:
            # dynamic objects
            instances_list = light['Frames'][frame_id]['Instances']
        else:
            # no static or dynamic instance to build
            continue

        light_name = light['Name']
        light_type = light['Type']
        assert light_type in ('Ambient', 'PointLight', 'Envmap',
                              'DirectionalLight')

        # loop over light instances
        for inst_cnt, light_instance in enumerate(instances_list, start=1):
            inst_name = f'{light_name}_inst_{inst_cnt}'
            if light_type == 'PointLight':
                scene_dict[inst_name] = {
                    'type': 'point',
                    'position': light_instance['Position'],
                    'intensity': {
                        'type': 'spectrum',
                        'value': light_instance['Intensity'],
                    }
                }
            elif light_type == 'Envmap':
                scene_dict[inst_name] = {
                    'type': 'envmap',
                    'filename': light['File'],
                    'to_world': mi.ScalarTransform4f(
                                    euler_to_mat(light_instance['Rotate'])),
                    'scale':    light_instance.get('Intensity') or 1.0
                 }
            elif light_type == 'DirectionalLight':
                scene_dict[inst_name] = {
                    'type': 'directional',
                    'direction': light_instance['Direction'],
                    'irradiance': {
                        'type': 'rgb',
                        'value': light_instance['Irradiance'],
                    }
                 }
            else:  # 'Ambient'
                scene_dict[inst_name] = {
                    'type': 'constant',
                    'radiance': {
                        'type': 'rgb',
                        'value': light_instance['Intensity'],
                    }
                 }

            lights_inst_cnt += inst_cnt

    return lights_inst_cnt

##########################################################

def add_object_instances(scene_dict, json_dict, static, frame_id=None):
    """
    Add object instances (composed of meshes instances) to the scene
    dictionnary.

    Arguments:
    - scene_dict: the scene dictionnary (dict)
    - json_dict: content of the JSON file (dict)
    - static: static/dynamic part of the scene (True/False)
    - frame_id: id of the frame for dynamic part of the scene (int)

    Return the number of meshes instances created.
    """
    assert static in (True, False)

    inst_cnt  = 0
    # loop over scene objects
    for obj in json_dict['ObjectsInstances']:
        # retrieve the list of instances to create
        if static and 'Instances' in obj:
            # static objects
            instances_list = obj['Instances']
        elif (not static) and 'Frames' in obj:
            # dynamic objects
            instances_list = obj['Frames'][frame_id]['Instances']
        else:
            # no static or dynamic instance to build
            continue

        obj_name = obj['Ref']

        # loop over object instances
        for obj_inst_cnt, obj_instance in enumerate(instances_list, start=1):
            # compute object instance transfo
            inst_transfo = compute_transfo(obj_instance['Translate'],
                                           obj_instance['Rotate'],
                                           obj_instance['Scale'])
            # add object instance to scene
            inst_name = f'{obj_name}_inst_{obj_inst_cnt}'
            
            while True:
                if (inst_name in scene_dict):
                  obj_inst_cnt +=1
                  inst_name = f'{obj_name}_inst_{obj_inst_cnt}'
                else:
                  break
            
            
            scene_dict[inst_name] = {
                'type': 'instance',
                'to_world': inst_transfo,
                'shapegroup': {
                    'type': 'ref',
                    'id': f'sh_grp_{obj_name}'
                }
            }
            #inst_cnt += 1

    return inst_cnt

##########################################################

def make_scene_dict(json_dict, scene_dict_static, frame_id):
    """
    Create the complete scene dictionnary for Mitsuba3, taking as input the
    content of a JSON file, and the static part of the scene.

    Arguments:
    - json_dict: content of the JSON file (dict)
    - scene_dict_static: dictionnary of the static part of the scene (dict)
    - frame_id: the id of the frame (int)

    Return the scene dictionnary appropriate for Mitsuba3 (dict), and
    and a dict of instances, lights and cameras numbers.
    """
    scene_dict = dict(scene_dict_static)  # deep copy

    # add dynamic objects
    inst_cnt = add_object_instances(scene_dict, json_dict, static=False,
                                    frame_id=frame_id)

    # add dynamic lights
    lights_cnt = add_lights(scene_dict, json_dict, static=False,
                            frame_id=frame_id)

    # add dynamic cameras
    cameras_cnt = add_cameras(scene_dict, json_dict, static=False,
                  frame_id=frame_id)

    #DBG print('scene_dict=', scene_dict)
    return scene_dict, {'instances_cnt': inst_cnt, 'lights_cnt': lights_cnt,
                        'cameras_cnt': cameras_cnt }

##########################################################

def main(json_filepath):
    """
    Render the 3D scene discribed in a JSON file with Mitsuba3.

    Arguments:
    - json_filepath: path to JSON file (str)
    """

    # load JSON
    with open(json_filepath, encoding='utf-8') as file:
        json_dict = json.load(file)
    fix_files_path(json_dict, json_filepath)

    # process imports
    merge_imports(json_dict)
    #DBG fprint('json_dict =') ; fprint(json_dict)

    # prune unused Scene Objects
    prune_objects(json_dict)

    # create scene
    backend = json_dict['Scene'].get('M3Backend') or 'scalar_rgb'
    fprint(f'Using "{backend}" backend.')
    mi.set_variant(backend)
    threads = json_dict['Scene'].get('Threads') or -1
    if threads >= 2:
        # set thread number taking into account the extra default thread
        drjit.set_thread_count(threads-1)
    fprint(f'Threads count set to {threads}.')

    start = time.time_ns()
    shapegroups, load_cnt = load_meshes(json_dict)
    elapsed = seconds_since(start)
    fprint(f'{load_cnt} mesh(es) loaded in {elapsed:.3f}s.')
    #DBG print('shapegroups=', shapegroups, flush=True) #DBG

    start = time.time_ns()
    scene_dict_static, numbers = \
        make_scene_dict_static(shapegroups, json_dict)
    elapsed = seconds_since(start)
    fprint(f'{numbers["instances_cnt"]} objects instance(s) added to static scene dict.')
    fprint(f'{numbers["lights_cnt"]} light(s) added to static scene dict.')
    fprint(f'{numbers["cameras_cnt"]} camera(s) added to static scene dict.')
    fprint(f'Static scene dict built in {elapsed:.3f}s.')
    #DBG print('scene_dict_static=', scene_dict_static)

    # retrieve scene info
    spp = json_dict['Scene'].get('Quality') or 128
    fprint(f'Number of samples per point set to {spp}.')
    scene_name = json_dict['Scene'].get('Name')

    # render all frames
    first_frame, last_frame = json_dict['Scene']['Frames']
    for frame_id in range(first_frame, last_frame+1):
        fprint(f'\n* Frame {frame_id}')

        start = time.time_ns()
        scene_dict, numbers = \
            make_scene_dict(json_dict, scene_dict_static, frame_id)
        elapsed = seconds_since(start)
        fprint(f'{numbers["instances_cnt"]} object instance(s) added to scene dict in {elapsed:.3f}s.')
        fprint(f'{numbers["lights_cnt"]} light(s) added to scene dict in {elapsed:.3f}s.')
        fprint(f'{numbers["cameras_cnt"]} camera(s) added to scene dict in {elapsed:.3f}s.')
        fprint(f'Dynamic scene dict built in {elapsed:.3f}s.')
        #DBG fprint('scene_dict =') ; fprint(scene_dict)

        start = time.time_ns()
        scene = mi.load_dict(scene_dict)
        elapsed = seconds_since(start)
        fprint(f'Scene loaded in Mitsuba3 in {elapsed:.3f}s.')
        #DBG print('scene =') ; print(scene)

        #print(f'Memory usage after loading scene: {resource.getrusage(resource.RUSAGE_SELF).ru_maxrss} kB')
        # peak memory usage (kilobytes on Linux, bytes on OS X)

        # render for each camera
        camera_nbr = \
            len([x for x in scene_dict.keys() if x.startswith('sensor__')])
        for camera_id in range(camera_nbr):
            start = time.time_ns()
            img = mi.render(scene, spp=spp, sensor=camera_id)
            elapsed = seconds_since(start)
            fprint(f'Scene rendered with camera {camera_id} in {elapsed:.3f}s.')

            # write image to file
            timestamp = time.strftime("%y%m%d-%H%M")
            frame_id_str = str(frame_id).rjust(4, '0')
            #img_filename = f'{scene_name}_cam-{camera_id}.png'
            #img_filename = f'{scene_name}_{timestamp}_cam-{camera_id}_frame-{frame_id_str}.png'
            img_filename = f'{scene_name}_cam-{camera_id}_frame-{frame_id_str}.png'
            mi.util.write_bitmap(img_filename, img)
            fprint(f'Image saved to file "{img_filename}".')

            # display image
            #show_image(img)
        
        gc.collect(generation=2)

    # show all images
    #plt.show()

##########################################################

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("json_filepath", help="path to JSON file")
    args = parser.parse_args()
    main(args.json_filepath)
