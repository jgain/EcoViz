/*******************************************************************************
 *
 * EcoSynth - Data-driven Authoring of Large-Scale Ecosystems (Undergrowth simulator)
 * Copyright (C) 2020  J.E. Gain  (jgain@cs.uct.ac.za)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ********************************************************************************/


// constraint.cpp:
// author: James Gain
// date: 5 November 2013
//       21 January 2013 - curve constraints

#include "shape.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//
// Shape
//

void Shape::setColour(GLfloat * col)
{
    int i;

    for(i = 0; i < 4; i++)
        diffuse[i] = col[i];
    for(i = 0; i < 3; i++)
        ambient[i] = diffuse[i] * 0.8f; // * 0.75f
    ambient[3] = diffuse[3];
    for(i = 0; i < 3; i++)
        specular[i] = std::min(1.0f, diffuse[i] * 0.3f); // * 1.25f
    specular[3] = diffuse[3];
}

void Shape::genCylinder(float radius, float height, int slices, int stacks, glm::mat4x4 trm)
{
    int i, j, base;

    float a, x, y, h = 0.0f;
    float stepa = PI2 / (float) slices;
    float stepz = height / (float) stacks;
    glm::vec4 p;
    glm::vec3 v;

    base = int(verts.size()) / 8;
    for(i = 0; i <= stacks; i++)
    {
        a = 0.0f;
        for (j = 0; j < slices; j++)
        {
            x = cosf(a) * radius;
            y = sinf(a) * radius;

            // apply transformation
            p = trm * glm::vec4(x, y, h, 1.0f);
            v = glm::mat3(trm) * glm::normalize(glm::vec3(x, y, 0.0f));

            verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
            verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
            verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal

            if(i > 0)
            {
                if(j < slices-1)
                {
                    indices.push_back(base-slices+j); indices.push_back(base-slices+j+1);indices.push_back(base+j);
                    indices.push_back(base-slices+j+1); indices.push_back(base+j+1); indices.push_back(base+j);
                }
                else // wrap
                {
                    indices.push_back(base-slices+j); indices.push_back(base-slices); indices.push_back(base+j);
                    indices.push_back(base-slices); indices.push_back(base); indices.push_back(base+j);
                }
            }
            a += stepa;
        }
        base += slices;
        h += stepz;
    }
}


void Shape::genCappedCylinder(float startradius, float endradius, float height, int slices, int stacks, glm::mat4x4 trm, bool clip)
{
    int i, j, base;

    float a, x, y, h = 0.0f, radius;
    float stepa = PI2 / (float) slices;
    float stepz = height / (float) stacks;
    float stepr = (endradius - startradius) / (float) (stacks);
    glm::vec4 p;
    glm::vec3 v;

    base = int(verts.size()) / 8;
    radius = startradius;
    for(i = 0; i <= stacks; i++)
    {
        a = 0.0f;
        for (j = 0; j < slices; j++)
        {
            x = cosf(a) * radius;
            y = sinf(a) * radius;

            // apply transformation
            p = trm * glm::vec4(x, y, h, 1.0f);
            v = glm::mat3(trm) * glm::normalize(glm::vec3(x, y, 0.0f));

            verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
            verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
            verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal

            if(i > 0)
            {
                if(j < slices-1)
                {
                    indices.push_back(base-slices+j); indices.push_back(base-slices+j+1);indices.push_back(base+j);
                    indices.push_back(base-slices+j+1); indices.push_back(base+j+1); indices.push_back(base+j);
                }
                else // wrap
                {
                    indices.push_back(base-slices+j); indices.push_back(base-slices); indices.push_back(base+j);
                    indices.push_back(base-slices); indices.push_back(base); indices.push_back(base+j);
                }
            }
            a += stepa;
        }

        base += slices;
        h += stepz;
        radius += stepr;
    }

    // cap base
    // lid center and rim
    p = trm * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    v = glm::mat3(trm) * glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
    verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal

    a = 0.0f; radius = startradius;
    for (j = 0; j < slices; j++)
    {
        x = cosf(a) * radius;
        y = sinf(a) * radius;

        // apply transformation
        p = trm * glm::vec4(x, y, 0.0f, 1.0f);
        v = glm::mat3(trm) * glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));

        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
        verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal
        a += stepa;
    }

    // face indices
    base += slices;
    for (j = 0; j < slices; j++)
    {
        if(j < slices-1)
        {
            indices.push_back(base-slices+j+1);indices.push_back(base-slices+j); indices.push_back(base);
        }
        else // wrap
        {
            indices.push_back(base-slices); indices.push_back(base-slices+j);indices.push_back(base);
        }
    }

    base++;
    // cap lid
    // lid center and rim
    p = trm * glm::vec4(0.0f, 0.0f, height, 1.0f);
    v = glm::mat3(trm) * glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f));
    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
    verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal

    a = 0.0f; radius = endradius;
    for (j = 0; j < slices; j++)
    {
        x = cosf(a) * radius;
        y = sinf(a) * radius;

        // apply transformation
        p = trm * glm::vec4(x, y, height, 1.0f);
        v = glm::mat3(trm) * glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f));

        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
        verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal
        a += stepa;
    }

    // face indices
    base += slices;
    for (j = 0; j < slices; j++)
    {
        if(j < slices-1)
        {
            indices.push_back(base-slices+j); indices.push_back(base-slices+j+1);indices.push_back(base);
        }
        else // wrap
        {
            indices.push_back(base-slices+j); indices.push_back(base-slices); indices.push_back(base);
        }
    }
}

void Shape::genCappedCone(float startradius, float height, int slices, int stacks, glm::mat4x4 trm, bool clip)
{
    int i, j, base;

    float endradius = 0.001f;
    float a, x, y, h = 0.0f, radius;
    float stepa = PI2 / (float) slices;
    float stepz = height / (float) stacks;
    float stepr = (endradius - startradius) / (float) (stacks);
    glm::vec4 p;
    glm::vec3 v;

    base = int(verts.size()) / 8;
    radius = startradius;
    for(i = 0; i <= stacks; i++)
    {
        a = 0.0f;
        for (j = 0; j < slices; j++)
        {
            x = cosf(a) * radius;
            y = sinf(a) * radius;

            // apply transformation
            p = trm * glm::vec4(x, y, h, 1.0f);
            v = glm::mat3(trm) * glm::normalize(glm::vec3(x, y, 0.0f));

            verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
            verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
            verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal

            if(i > 0)
            {
                if(j < slices-1)
                {
                    indices.push_back(base-slices+j); indices.push_back(base-slices+j+1);indices.push_back(base+j);
                    indices.push_back(base-slices+j+1); indices.push_back(base+j+1); indices.push_back(base+j);
                }
                else // wrap
                {
                    indices.push_back(base-slices+j); indices.push_back(base-slices); indices.push_back(base+j);
                    indices.push_back(base-slices); indices.push_back(base); indices.push_back(base+j);
                }
            }
            a += stepa;
        }

        base += slices;
        h += stepz;
        radius += stepr;
    }

    // cap base
    // lid center and rim
    p = trm * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    v = glm::mat3(trm) * glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
    verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal

    a = 0.0f; radius = startradius;
    for (j = 0; j < slices; j++)
    {
        x = cosf(a) * radius;
        y = sinf(a) * radius;

        // apply transformation
        p = trm * glm::vec4(x, y, 0.0f, 1.0f);
        v = glm::mat3(trm) * glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));

        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
        verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal
        a += stepa;
    }

    // face indices
    base += slices;
    for (j = 0; j < slices; j++)
    {
        if(j < slices-1)
        {
            indices.push_back(base-slices+j+1);indices.push_back(base-slices+j); indices.push_back(base);
        }
        else // wrap
        {
            indices.push_back(base-slices); indices.push_back(base-slices+j);indices.push_back(base);
        }
    }
}

void Shape::genPyramid(float baselen, float toplen, float height, glm::mat4x4 trm)
{
    int i, base;
    Vector v;
    glm::vec4 p;
    glm::vec3 n;

    // base verts
    vpPoint b[4];
    b[0] = vpPoint(-baselen/2.0f, -baselen/2.0f, 0.0f);
    b[1] = vpPoint(baselen/2.0f, -baselen/2.0f, 0.0f);
    b[2] = vpPoint(baselen/2.0f, baselen/2.0f, 0.0f);
    b[3] = vpPoint(-baselen/2.0f, baselen/2.0f, 0.0f);

    // top verts
    vpPoint t[4];
    t[0] = vpPoint(-toplen/2.0f, -toplen/2.0f, height);
    t[1] = vpPoint(toplen/2.0f, -toplen/2.0f, height);
    t[2] = vpPoint(toplen/2.0f, toplen/2.0f, height);
    t[3] = vpPoint(-toplen/2.0f, toplen/2.0f, height);

    base = int(verts.size()) / 8;

    // base vertices
    v = Vector(0.0f, 0.0f, -1.0f);
    for(i = 0; i < 4; i++)
    {
        p = trm * glm::vec4(b[i].x, b[i].y, b[i].z, 1.0f);
        n = glm::mat3(trm) * glm::normalize(glm::vec3(v.i, v.j, v.k));
        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
        verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // normal
    }
    // counterclockwise winding
    indices.push_back(base+1); indices.push_back(base+0); indices.push_back(base+2);
    indices.push_back(base+0); indices.push_back(base+3); indices.push_back(base+2);
    base += 4;

    // top vertices
    v = Vector(0.0f, 0.0f, 1.0f);
    for(i = 0; i < 4; i++)
    {
        p = trm * glm::vec4(t[i].x, t[i].y, t[i].z, 1.0f);
        n = glm::mat3(trm) * glm::normalize(glm::vec3(v.i, v.j, v.k));
        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
        verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // normal
    }
    indices.push_back(base+2); indices.push_back(base+0); indices.push_back(base+1);
    indices.push_back(base+3); indices.push_back(base+0); indices.push_back(base+2);
    base += 4;

    // side vertices: -y
    v = Vector(0.0f, -height, baselen-toplen);
    v.normalize();
    vpPoint s[4];
    s[0] = b[0]; s[1] = b[1]; s[2] = t[1]; s[3] = t[0];
    for(i = 0; i < 4; i++)
    {
        p = trm * glm::vec4(s[i].x, s[i].y, s[i].z, 1.0f);
        n = glm::mat3(trm) * glm::normalize(glm::vec3(v.i, v.j, v.k));
        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
        verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // normal
    }
    indices.push_back(base+2); indices.push_back(base+0); indices.push_back(base+1);
    indices.push_back(base+3); indices.push_back(base+0); indices.push_back(base+2);
    base += 4;

    // side vertices: +y
    v = Vector(0.0f, height, baselen-toplen);
    v.normalize();
    s[0] = b[2]; s[1] = b[3]; s[2] = t[3]; s[3] = t[2];
    for(i = 0; i < 4; i++)
    {
        p = trm * glm::vec4(s[i].x, s[i].y, s[i].z, 1.0f);
        n = glm::mat3(trm) * glm::normalize(glm::vec3(v.i, v.j, v.k));
        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
        verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // normal
    }
    indices.push_back(base+2); indices.push_back(base+0); indices.push_back(base+1);
    indices.push_back(base+3); indices.push_back(base+0); indices.push_back(base+2);
    base += 4;

    // side vertices: +x
    v = Vector(height, 0.0f, baselen-toplen);
    v.normalize();
    s[0] = b[1]; s[1] = b[2]; s[2] = t[2]; s[3] = t[1];
    for(i = 0; i < 4; i++)
    {
        p = trm * glm::vec4(s[i].x, s[i].y, s[i].z, 1.0f);
        n = glm::mat3(trm) * glm::normalize(glm::vec3(v.i, v.j, v.k));
        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
        verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // normal
    }
    indices.push_back(base+2); indices.push_back(base+0); indices.push_back(base+1);
    indices.push_back(base+3); indices.push_back(base+0); indices.push_back(base+2);
    base += 4;

    // side vertices: -x
    v = Vector(-height, 0.0f, baselen-toplen);
    v.normalize();
    s[0] = b[3]; s[1] = b[0]; s[2] = t[0]; s[3] = t[3];
    for(i = 0; i < 4; i++)
    {
        p = trm * glm::vec4(s[i].x, s[i].y, s[i].z, 1.0f);
        n = glm::mat3(trm) * glm::normalize(glm::vec3(v.i, v.j, v.k));
        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
        verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // normal
    }
    indices.push_back(base+2); indices.push_back(base+0); indices.push_back(base+1);
    indices.push_back(base+3); indices.push_back(base+0); indices.push_back(base+2);
    base += 4;
}

void Shape::genSphereVert(float radius, float lat, float lon, glm::mat4x4 trm)
{

    float la, lo, x, y, z;
    glm::vec4 p;
    glm::vec3 v;

    la = PI+PI*lat;
    lo = PI2*lon;
    // this is unoptimized
    x = cosf(lo)*sinf(la)*radius;
    y = sinf(lo)*sinf(la)*radius;
    z = cosf(la)*radius;

    // apply transformation
    p = trm * glm::vec4(x, y, z, 1.0f);
    v = glm::mat3(trm) * glm::normalize(glm::vec3(x, y, z));

    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
    verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal
}

void Shape::genSphere(float radius, int slices, int stacks, glm::mat4x4 trm)
{
    int lat, lon, base;
    float plat, plon;

    // doesn't produce very evenly sized triangles, tend to cluster at poles
    base = int(verts.size()) / 8;
    for(lat = 0; lat <= stacks; lat++)
    {
        for(lon = 0; lon < slices; lon++)
        {
            plat = (float) lat / (float) stacks;
            plon = (float) lon / (float) slices;
            genSphereVert(radius, plat, plon, trm);

            if(lat > 0)
            {
                if(lon < slices-1)
                {
                    indices.push_back(base-slices+lon); indices.push_back(base-slices+lon+1); indices.push_back(base+lon);
                    indices.push_back(base-slices+lon+1); indices.push_back(base+lon+1); indices.push_back(base+lon);
                }
                else // wrap
                {
                    indices.push_back(base-slices+lon); indices.push_back(base-slices); indices.push_back(base+lon);
                    indices.push_back(base-slices); indices.push_back(base); indices.push_back(base+lon);
                }
            }
        }
        base += slices;
    }
}

void Shape::genTest()
{
    clear();

    // single triangle
    verts.push_back(-1.0f); verts.push_back(0.0f); verts.push_back(-1.0f); // position
    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
    verts.push_back(0.0f); verts.push_back(-1.0f); verts.push_back(0.0f); // normal

    verts.push_back(-1.0f); verts.push_back(0.0f); verts.push_back(1.0f); // position
    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
    verts.push_back(0.0f); verts.push_back(-1.0f); verts.push_back(0.0f); // normal

    verts.push_back(1.0f); verts.push_back(0.0f); verts.push_back(1.0f); // position
    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
    verts.push_back(0.0f); verts.push_back(-1.0f); verts.push_back(0.0f); // normal

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
}

void Shape::genCurve(std::vector<vpPoint> &curve, View * view, float thickness, float tol, bool closed, bool offset, bool viewadapt)
{

    // Double the number of vertices actually required. Consider compacting later.

    // This needs the view class because thickening happens orthogonal to the view direction
    int i, j, lim, numv;
    vpPoint s, e, p[4], m[4], os, oe, c;
    Vector v, n, pv, pn, eye, negeye, off;
    float width = thickness;
    bool firstseg = true;

    numv = int(verts.size()) / 8;
    if((int) curve.size() > 0)
    {
        eye = view->getDir(); eye.normalize();
        negeye = eye; negeye.mult(-1.0f); off = negeye; off.mult(0.005f);

        if(closed)
            lim = (int) curve.size();
        else
            lim = (int) curve.size()-1;

        s = curve[0];
        for(i = 0; i < lim; i++)
        {
            // current segment
            if(closed)
                e = curve[(i+1)%lim];
            else
                e = curve[i+1];


            if(offset) // shift points closer to the viewpoint
            {
                // shift higher above terrain
                off = Vector(0.0f, 0.001f, 0.0f);
                // c = view->getCOP();
                // off.diff(os, c); off.normalize(); off.mult(0.001f);
                off.pntplusvec(s, &os);
                off.pntplusvec(e, &oe);
            }
            else
            {
                os = s; oe = e;
            }
            v.diff(os, oe);
            if(v.length() > tol) // ignore points that are too close together
            {
                v.normalize();

                // find vector orthogonal to segment but parallel to the viewing plane
                n.cross(v, eye);
                n.normalize();
                if(viewadapt) // consider distance from viewpoint
                {
                    // broken - fix if needed
                    c = view->getCOP();
                    off.diff(c, os);
                    n.mult(width * (65.0f * off.length()));
                }
                else
                    n.mult(width);

                // construct line quad vertices
                n.pntplusvec(oe, &p[1]);
                n.pntplusvec(os, &p[0]);
                n.mult(-1.0f);
                n.pntplusvec(os, &p[3]);
                n.pntplusvec(oe, &p[2]);

                // find normal
                n = negeye;

                for(j = 0; j < 4; j++)
                {
                    verts.push_back(p[j].x); verts.push_back(p[j].y); verts.push_back(p[j].z); // position
                    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
                    verts.push_back(n.i); verts.push_back(n.j); verts.push_back(n.k); // normal
                }
                indices.push_back(numv+0); indices.push_back(numv+1); indices.push_back(numv+3);
                indices.push_back(numv+2); indices.push_back(numv+3); indices.push_back(numv+1);
                numv += 4;

                // mitres to previous segment
                if(firstseg)
                {
                    if(closed)
                    {
                        // calculate thickening for closing the loop
                        s = curve[(int) curve.size()-1]; e = curve[0];
                        v.diff(os, oe);
                        v.normalize();
                        n.cross(v, eye);
                        n.normalize();

                        if(viewadapt)
                        {
                            c = view->getCOP();
                            off.diff(c, os);
                            n.mult(width * (65.0f * off.length()));
                        }
                        else
                            n.mult(width);

                        n.pntplusvec(os, &m[0]);
                        n.mult(-1.0f);
                        n.pntplusvec(os, &m[1]);
                        m[2] = p[0]; m[3] = p[3];

                        n = negeye;
                        for(j = 0; j < 4; j++)
                        {
                            verts.push_back(m[j].x); verts.push_back(m[j].y); verts.push_back(m[j].z); // position
                            verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
                            verts.push_back(n.i); verts.push_back(n.j); verts.push_back(n.k); // normal
                        }
                        indices.push_back(numv+0); indices.push_back(numv+1); indices.push_back(numv+3);
                        indices.push_back(numv+2); indices.push_back(numv+3); indices.push_back(numv+1);
                        numv += 4;
                    }
                    firstseg = false;
                }
                else
                {
                    // m[1] and m[2] already stored from previous iteration
                    m[0] = p[0]; m[3] = p[3];
                    n = negeye;
                    verts.push_back(os.x); verts.push_back(os.y); verts.push_back(os.z); // position
                    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
                    verts.push_back(n.i); verts.push_back(n.j); verts.push_back(n.k); // normal
                    for(j = 0; j < 4; j++)
                    {
                        verts.push_back(m[j].x); verts.push_back(m[j].y); verts.push_back(m[j].z); // position
                        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
                        verts.push_back(n.i); verts.push_back(n.j); verts.push_back(n.k); // normal
                    }
                    indices.push_back(numv+0); indices.push_back(numv+2); indices.push_back(numv+1);
                    indices.push_back(numv+0); indices.push_back(numv+4); indices.push_back(numv+3);
                    numv += 5;

                }
                m[1] = p[1];
                m[2] = p[2];
                s = e;
            }
        }
    }
}

void Shape::genCylinderCurve(std::vector<vpPoint> &curve, float radius, float tol, int slices)
{
    int i, j, lim, base;
    vpPoint s, e;
    Vector v, vfin, vstart, vrot;
    glm::mat4x4 idt, tfm;
    glm::vec3 trs, n, rot;
    float angle, a, x, y, stepa = PI2 / (float) slices;
    glm::vec4 p;

    base = int(verts.size()) / 8;
    if((int) curve.size() > 1)
    {
        lim = (int) curve.size()-1;
        s = curve[0]; s.y += 0.05f;
        for(i = 0; i < lim; i++)
        {
            e = curve[i+1]; e.y += 0.05f;
            v.diff(s, e);

            if(i == 0) // base stack
            {
                // translate to s
                idt = glm::mat4(1.0f);
                trs = glm::vec3(s.x, s.y, s.z);
                tfm = glm::translate(idt, trs);

                // azimuth rotation
                vfin = v;
                vfin.j = 0.0f; vfin.normalize();
                angle = RAD2DEG * atan2(vfin.k, vfin.i);
                if(angle < 0.0f)
                    angle += 360.0f;
                rot = glm::vec3(0.0f, -1.0f, 0.0f);
                tfm = glm::rotate(tfm, glm::radians(angle), rot);

                // elevation rotation
                /*
                 v.normalize();
                 rot = glm::vec3(-1.0f, 0.0f, 0.0f);
                 angle = RAD2DEG * acosf(v.dot(vfin));
                 tfm = glm::rotate(tfm, glm::radians(angle), rot);*/

                // create cylinder from s to e
                // genCylinder(radius, v.length(), slices, 1, tfm);

                // single cylinder stack linking to previous stack as necessary
                a = 0.0f;
                for (j = 0; j < slices; j++)
                {
                    x = cosf(a) * radius;
                    y = sinf(a) * radius;

                    p = tfm * glm::vec4(0.0f, y, x, 1.0f);
                    n = glm::mat3(tfm) * glm::normalize(glm::vec3(0.0f, y, x));

                    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
                    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
                    verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // normal
                    a -= stepa;
                }
                base += slices;
            }
            else
            {
                if(v.length() > tol || i == lim-1) // ignore points that are too close together, except for last segment
                {
                    // translate to e
                    idt = glm::mat4(1.0f);
                    trs = glm::vec3(e.x, e.y, e.z);
                    tfm = glm::translate(idt, trs);

                    // azimuth rotation
                    vfin = v;
                    vfin.j = 0.0f; vfin.normalize();
                    angle = RAD2DEG * atan2(vfin.k, vfin.i);
                    if(angle < 0.0f)
                        angle += 360.0f;
                    rot = glm::vec3(0.0f, -1.0f, 0.0f);
                    tfm = glm::rotate(tfm, glm::radians(angle), rot);

                    // elevation rotation
                    /*
                     v.normalize();
                     rot = glm::vec3(-1.0f, 0.0f, 1.0f);
                     angle = RAD2DEG * acosf(v.dot(vfin));
                     tfm = glm::rotate(tfm, glm::radians(angle), rot);*/


                    // single cylinder stack linking to previous stack as necessary
                    a = 0.0f;
                    for (j = 0; j < slices; j++)
                    {
                        x = cosf(a) * radius;
                        y = sinf(a) * radius;

                        p = tfm * glm::vec4(0.0f, y, x, 1.0f);
                        n = glm::mat3(tfm) * glm::normalize(glm::vec3(0.0f, y, x));

                        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
                        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
                        verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // normal

                        if(i > 0)
                        {
                            if(j < slices-1)
                            {
                                indices.push_back(base-slices+j); indices.push_back(base-slices+j+1);indices.push_back(base+j);
                                indices.push_back(base-slices+j+1); indices.push_back(base+j+1); indices.push_back(base+j);
                            }
                            else // wrap
                            {
                                indices.push_back(base-slices+j); indices.push_back(base-slices); indices.push_back(base+j);
                                indices.push_back(base-slices); indices.push_back(base); indices.push_back(base+j);
                            }
                        }
                        a -= stepa;
                    }
                    base += slices;
                    s = e;
                }
            }
        }
    }
}

void Shape::genDashedCylinderCurve(std::vector<vpPoint> &curve, float radius, float tol, float dashlen, int slices)
{
    int i, j, lim, base;
    vpPoint s, e;
    Vector v, vfin, vstart, vrot;
    glm::mat4x4 idt, tfm;
    glm::vec3 trs, n, rot;
    float angle, a, x, y, stepa = PI2 / (float) slices, dashaccum = 0.0f;
    glm::vec4 p;
    bool dashon = true;

    base = int(verts.size()) / 8;
    if((int) curve.size() > 1)
    {
        lim = (int) curve.size()-1;
        s = curve[0]; s.y += 0.05f;
        for(i = 0; i < lim; i++)
        {
            e = curve[i+1]; e.y += 0.05f;
            v.diff(s, e);

            if(i == 0) // base stack
            {
                // translate to s
                idt = glm::mat4(1.0f);
                trs = glm::vec3(s.x, s.y, s.z);
                tfm = glm::translate(idt, trs);

                // azimuth rotation
                vfin = v;
                vfin.j = 0.0f; vfin.normalize();
                angle = RAD2DEG * atan2(vfin.k, vfin.i);
                if(angle < 0.0f)
                    angle += 360.0f;
                rot = glm::vec3(0.0f, -1.0f, 0.0f);
                tfm = glm::rotate(tfm, glm::radians(angle), rot);

                // single cylinder stack linking to subsequent stacks as necessary
                a = 0.0f;
                for (j = 0; j < slices; j++)
                {
                    x = cosf(a) * radius;
                    y = sinf(a) * radius;

                    p = tfm * glm::vec4(0.0f, y, x, 1.0f);
                    n = glm::mat3(tfm) * glm::normalize(glm::vec3(0.0f, y, x));

                    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
                    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
                    verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // normal
                    a -= stepa;
                }
                base += slices;
            }
            else
            {
                if(v.length() > tol || i == lim-1) // ignore points that are too close together, except for last segment
                {
                    dashaccum += v.length();
                    if(dashaccum >= dashlen) // toggle whether or not to draw dash
                    {
                        dashon = !dashon;
                        dashaccum = 0.0f;
                    }

                    // TO DO: cap cylinder

                    // translate to e
                    idt = glm::mat4(1.0f);
                    trs = glm::vec3(e.x, e.y, e.z);
                    tfm = glm::translate(idt, trs);

                    // azimuth rotation
                    vfin = v;
                    vfin.j = 0.0f; vfin.normalize();
                    angle = RAD2DEG * atan2(vfin.k, vfin.i);
                    if(angle < 0.0f)
                        angle += 360.0f;
                    rot = glm::vec3(0.0f, -1.0f, 0.0f);
                    tfm = glm::rotate(tfm, glm::radians(angle), rot);

                    // elevation rotation
                    /*
                     v.normalize();
                     rot = glm::vec3(-1.0f, 0.0f, 1.0f);
                     angle = RAD2DEG * acosf(v.dot(vfin));
                     tfm = glm::rotate(tfm, angle, rot);*/


                    // single cylinder stack linking to previous stack as necessary
                    a = 0.0f;
                    for (j = 0; j < slices; j++)
                    {
                        x = cosf(a) * radius;
                        y = sinf(a) * radius;

                        p = tfm * glm::vec4(0.0f, y, x, 1.0f);
                        n = glm::mat3(tfm) * glm::normalize(glm::vec3(0.0f, y, x));

                        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
                        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
                        verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // normal

                        if(i > 0 && dashon)
                        {
                            if(j < slices-1)
                            {
                                indices.push_back(base-slices+j); indices.push_back(base-slices+j+1);indices.push_back(base+j);
                                indices.push_back(base-slices+j+1); indices.push_back(base+j+1); indices.push_back(base+j);
                            }
                            else // wrap
                            {
                                indices.push_back(base-slices+j); indices.push_back(base-slices); indices.push_back(base+j);
                                indices.push_back(base-slices); indices.push_back(base); indices.push_back(base+j);
                            }
                        }
                        a -= stepa;
                    }
                    base += slices;
                    s = e;
                }
            }
        }
    }
}

void Shape::genSphereCurve(std::vector<vpPoint> &curve, float thickness)
{
    int i;
    glm::mat4 tfm, idt;
    glm::vec3 trs;

    // assume view transformations are set up correctly
    if((int) curve.size() > 0)
    {
        for(i = 0; i < (int) curve.size(); i++)
        {

            idt = glm::mat4(1.0f);
            trs = glm::vec3(curve[i].x, curve[i].y, curve[i].z);
            tfm = glm::translate(idt, trs);
            genSphere(thickness, 10, 10, tfm);
        }
    }
}

void Shape::genHemisphereVert(float radius, float lat, float lon, glm::mat4x4 trm)
{
    float la, lo, x, y, z;
    glm::vec4 p;
    glm::vec3 v;

    la = PI+0.5f*PI*lat;
    lo = PI2*lon;
    // this is unoptimized
    x = cosf(lo)*sinf(la)*radius;
    y = sinf(lo)*sinf(la)*radius;
    z = cosf(la)*radius;

    // apply transformation
    p = trm * glm::vec4(x, y, z, 1.0f);
    v = glm::mat3(trm) * glm::normalize(glm::vec3(x, y, z));

    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
    verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal
}

void Shape::genHemisphere(float radius, int slices, int stacks, glm::mat4x4 trm)
{
    int lat, lon, base;
    float plat, plon;

    // doesn't produce very evenly sized triangles, tend to cluster at poles
    base = int(verts.size()) / 8;
    for(lat = 0; lat <= stacks; lat++)
    {
        for(lon = 0; lon < slices; lon++)
        {
            plat = (float) lat / (float) stacks;
            plon = (float) lon / (float) slices;
            genHemisphereVert(radius, plat, plon, trm);

            if(lat > 0)
            {
                if(lon < slices-1)
                {
                    indices.push_back(base-slices+lon); indices.push_back(base-slices+lon+1); indices.push_back(base+lon);
                    indices.push_back(base-slices+lon+1); indices.push_back(base+lon+1); indices.push_back(base+lon);
                }
                else // wrap
                {
                    indices.push_back(base-slices+lon); indices.push_back(base-slices); indices.push_back(base+lon);
                    indices.push_back(base-slices); indices.push_back(base); indices.push_back(base+lon);
                }
            }
        }
        base += slices;
    }

}

static std::ostream & operator << (std::ostream &o, const vpPoint &rhs)
{
    o << rhs.x << ", " << rhs.y << ", " << rhs.z << std::endl;
    return o;
}

void Shape::genPlane(const vpPoint &orient, const vpPoint &center, float thickness, float length, glm::mat4x4 trm)
{
    vpPoint b[4];
    glm::vec4 p;
    Vector v;
    glm::vec3 n;

    glm::vec3 cent = glm::vec3(center.x, center.y, center.z);
    glm::vec3 ornt = glm::vec3(orient.x, orient.y, orient.z);
    ornt = glm::normalize(ornt);
    glm::vec3 orthog = glm::cross(glm::vec3(0.0f, -1.0f, 0.0f), ornt);

    glm::vec3 tl = cent + ornt * length / 2.0f - orthog * thickness;
    glm::vec3 tr = cent + ornt * length / 2.0f + orthog * thickness;
    glm::vec3 bl = cent - ornt * length / 2.0f - orthog * thickness;
    glm::vec3 br = cent - ornt * length / 2.0f + orthog * thickness;

    //std::cout << tl << std::endl;

    b[0] = vpPoint(bl.x, 1.0f, bl.z);
    b[1] = vpPoint(tl.x, 1.0f, tl.z);
    b[2] = vpPoint(tr.x, 1.0f, tr.z);
    b[3] = vpPoint(br.x, 1.0f, br.z);

    /*
    std::cout << b[0] << std::endl;
    std::cout << b[1] << std::endl;
    std::cout << b[2] << std::endl;
    std::cout << b[3] << std::endl;
*/
    int base = int(verts.size()) / 8;

    // base vertices
    v = Vector(0.0f, 1.0f, 0.0f);
    //v = Vector(0.0f, 0.0f, -1.0f);
    for(int i = 0; i < 4; i++)
    {
        p = trm * glm::vec4(b[i].x, b[i].y, b[i].z, 1.0f);
        n = glm::mat3(trm) * glm::normalize(glm::vec3(v.i, v.j, v.k));
        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
        verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z); // normal
        // std::cout << "(p.x, p.y, p.z) = " << p.x << "," << p.y << "," << p.z << ")\n";
        // std::cout << "(n.x, n.y, n.z) = " << n.x << "," << n.y << "," << n.z << ")\n";
    }
    // counterclockwise winding
    indices.push_back(base+1); indices.push_back(base+0); indices.push_back(base+2);
    indices.push_back(base+0); indices.push_back(base+3); indices.push_back(base+2);
    base += 4;

}

ShapeDrawData Shape::getDrawParameters()
{
    ShapeDrawData sdd;

    sdd.VAO = vaoConstraint;
    for(int i = 0; i < 4; i++)
        sdd.diffuse[i] = diffuse[i];
    for(int i = 0; i < 4; i++)
        sdd.specular[i] = specular[i];
    for(int i = 0; i < 4; i++)
        sdd.ambient[i] = ambient[i];
    sdd.indexBufSize = (int) indices.size();
    sdd.numInstances = numInstances;
    sdd.texID = 0;
    sdd.current = false; // default setting

    return sdd;
}


bool Shape::bindInstances(std::vector<glm::vec3> * iTransl, std::vector<glm::vec2> * iScale, std::vector<float> * icols)
{

    size_t bytesBound = 0;

    QOpenGLExtraFunctions *ef = QOpenGLContext::currentContext()->extraFunctions();

    if((int) indices.size() > 0 && ((int) iTransl->size() == (int) icols->size()))
    {
        if (vboConstraint != 0)
        {

            ef->glDeleteVertexArrays(1, &vaoConstraint);
            ef->glDeleteBuffers(1, &vboConstraint);
            ef->glDeleteBuffers(1, &iboConstraint);
            //glDeleteBuffers(1, &iBuffer);
            ef->glDeleteBuffers(1, &iTranslBuffer);
            ef->glDeleteBuffers(1, &iScaleBuffer);
            ef->glDeleteBuffers(1, &cBuffer);
            vaoConstraint = 0;
            vboConstraint = 0;
            iboConstraint = 0;
            //iBuffer = 0;
            iTranslBuffer = 0;
            iScaleBuffer = 0;
            cBuffer = 0;
        }

        // vao
        ef->glGenVertexArrays(1, &vaoConstraint);
        ef->glBindVertexArray(vaoConstraint);

        // vbo
        // set up vertex buffer and copy in data
        ef->glGenBuffers(1, &vboConstraint);
        ef->glBindBuffer(GL_ARRAY_BUFFER, vboConstraint);
        ef->glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*(int) verts.size(), (GLfloat *) &verts[0], GL_STATIC_DRAW);

        bytesBound += sizeof(GLfloat)*(int) verts.size();

        // ibo
        ef->glGenBuffers(1, &iboConstraint);
        ef->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboConstraint);
        ef->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*(int) indices.size(), (GLuint *) &indices[0], GL_STATIC_DRAW);

        bytesBound += sizeof(GLuint) * (int)indices.size();

        // enable position attribute
        ef->glEnableVertexAttribArray(0);
        ef->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (void*)(0));

        // enable texture coord attribute
        const int sz = 3*sizeof(GLfloat);
        ef->glEnableVertexAttribArray(1);
        ef->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (void*)(sz) );

        // enable normals
        const int nz = 5*sizeof(GLfloat);
        ef->glEnableVertexAttribArray(2);
        ef->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (void*)(nz) );

        // generate buffer for translation and scaling (2 scales only)

        ef->glGenBuffers(1, &iTranslBuffer);
        ef->glBindBuffer(GL_ARRAY_BUFFER, iTranslBuffer);
        if((int) iTransl->size() > 0) // load instance data
        {
            numInstances = (int) iTransl->size();
            ef->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numInstances, (GLfloat *) & (* iTransl)[0], GL_DYNAMIC_DRAW);

            bytesBound  += sizeof(glm::vec3) * numInstances;
        }
        else // create a single instance
        {
            numInstances = 1;
            //std::vector<glm::mat4> tmpform;
            //glm::mat4 idt = glm::mat4(1.0f);
            //tmpform.push_back(idt);
            glm::vec3 tmpform = {0.0, 0.0, 0.0};

            ef->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numInstances, (GLfloat *) &tmpform[0], GL_DYNAMIC_DRAW);

            bytesBound += sizeof(glm::vec3) * numInstances;
        }

        ef->glGenBuffers(1, &iScaleBuffer);
        ef->glBindBuffer(GL_ARRAY_BUFFER, iScaleBuffer);
        if((int) iScale->size() > 0) // load instance data
        {
            numInstances = (int) iScale->size();
            ef->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * numInstances, (GLfloat *) & (* iScale)[0], GL_DYNAMIC_DRAW);

            bytesBound  += sizeof(glm::vec2) * numInstances;
        }
        else // create a single instance
        {
            numInstances = 1;
            //std::vector<glm::mat4> tmpform;
            //glm::mat4 idt = glm::mat4(1.0f);
            //tmpform.push_back(idt);
            glm::vec2 tmpform = {1.0, 1.0};

            ef->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * numInstances, (GLfloat *) &tmpform[0], GL_DYNAMIC_DRAW);

            bytesBound += sizeof(glm::vec2) * numInstances;
        }

        // set up vert atributes and instancing step
        ef->glBindBuffer(GL_ARRAY_BUFFER, iTranslBuffer);
        ef->glEnableVertexAttribArray(3);
        ef->glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const GLvoid*)(0) );
        ef->glVertexAttribDivisor(3, 1);

        ef->glBindBuffer(GL_ARRAY_BUFFER, iScaleBuffer);
        ef->glEnableVertexAttribArray(4);
        ef->glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (const GLvoid*)(0));
        ef->glVertexAttribDivisor(4, 1);


        // we need a full mat4 because the plant dimensions (non-uniform scale) as well as position are being instanced
        /*
        glGenBuffers(1, &iBuffer); // create a vertex buffer object for plant transform instancing
        glBindBuffer(GL_ARRAY_BUFFER, iBuffer);
        if((int) iforms->size() > 0) // load instance data
        {
            numInstances = (int) iforms->size();
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * numInstances, (GLfloat *) & (* iforms)[0], GL_DYNAMIC_DRAW);
        }
        else // create a single instance
        {
            numInstances = 1;
            std::vector<glm::mat4> tmpform;
            glm::mat4 idt = glm::mat4(1.0f);
            tmpform.push_back(idt);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * numInstances, (GLfloat *) &tmpform[0], GL_DYNAMIC_DRAW);
        }

        glBindBuffer(GL_ARRAY_BUFFER, iBuffer);
        for (unsigned int i = 0; i < 4 ; i++) {
            glEnableVertexAttribArray(3 + i);
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                                  (const GLvoid*)(sizeof(GLfloat) * i * 4));
            glVertexAttribDivisor(3 + i, 1);
        }
        */

        ef->glGenBuffers(1, &cBuffer); // create a vertex buffer object for plant colour instancing
        ef->glBindBuffer(GL_ARRAY_BUFFER, cBuffer);
        // colour buffer to allow subtle variations in plant colour
        if((int) icols->size() > 0)
        {
            numInstances = (int) icols->size();
            ef->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numInstances, (GLfloat *) & (* icols)[0], GL_DYNAMIC_DRAW);

            bytesBound += sizeof(float) * numInstances;
        }
        else // a single colour instance, with no change to the underlying colour
        {
            numInstances = 1;
            std::vector<float> tmpcol;
            //glm::vec4 idt = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            tmpcol.push_back(float(0.0));
            ef->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numInstances, (GLfloat *) &tmpcol[0], GL_DYNAMIC_DRAW);

            bytesBound += sizeof(float) * numInstances;
        }

        ef->glBindBuffer(GL_ARRAY_BUFFER, cBuffer);
        ef->glEnableVertexAttribArray(5);
        ef->glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0); // stride may need adjusting here
        // glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (const GLvoid*)0);
        ef->glVertexAttribDivisor(5, 1);

        ef->glBindVertexArray(0);

        return true;
    }
    else
    {
        // cerr << "indices = " << (int) indices.size() << " iforms = " << (int) iforms->size() << " icols = " << (int) icols->size() << endl;
        return false;
    }


}
