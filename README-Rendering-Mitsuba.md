
````markdown
# 🌄 Scene Rendering Project with Mitsuba

This project allows rendering 3D scenes exported from **Ecoviz** using **Mitsuba**, a physically-based renderer, together with Python tools for scene processing and visualization.

It reads scene data from a JSON file and produces realistic rendered images with physically-accurate lighting and materials.

---

## 🧠 What Is Mitsuba?

[Mitsuba](https://www.mitsuba-renderer.org/) is a **physically-based rendering (PBR)** system used for scientific visualization and research in light transport.  
It simulates real-world light interactions between materials, enabling the production of highly realistic renders.

In this project, Mitsuba is used as the rendering backend to visualize 3D environments exported from **Ecoviz**.

---

## 🧩 Prerequisites

Before running this project, ensure you have the following installed:

- **Mitsuba 3** (or newer):  
  Install and configure Mitsuba according to its [official installation guide](https://www.mitsuba-renderer.org/).  
  You can verify the installation by running:
  ```sh
  mitsuba --version
````

or in Python:

```python
import mitsuba
print(mitsuba.variant())
```

* **Python ≥ 3.8**
  Check your version:

  ```sh
  python --version
  ```

* **pip** (Python package manager)
  Usually installed automatically with Python.

---

## ⚙️ Installing Python Dependencies

Install the required Python libraries using the provided `requirements.txt` file:

```sh
pip install -r requirements.txt
```

### Example `requirements.txt`

```txt
mitsuba
numpy
matplotlib
```

---

## 🚀 Usage

To render a scene from a JSON file, run:

```sh
python render_scene.py path_to_json_file
```

### Example

```sh
python render_scene.py scene.json
```

This script performs the following:

1. Loads the scene description from the JSON file exported by Ecoviz.
2. Parses materials, geometry, lighting, and camera settings.
3. Calls Mitsuba to render the scene and produce an output image (e.g., `render.png`).

---

## 🧱 JSON Files

The JSON scene files are **automatically generated** by the **Ecoviz** C++ application using the **Export** button.
Each file contains all data necessary for rendering with Mitsuba, including:

* Scene geometry
* Material and texture layers
* Lighting setup
* Camera position and orientation

These parameters are fully compatible with Mitsuba’s Python interface.

---

## 🏞️ Terrain Texture Configuration

The **terrain material** in Ecoviz scenes is made of **multiple blended texture layers**, each representing a specific surface type (e.g., grass, soil, or rock).
Each layer defines its own texture images and a physical scale in meters.

### Example: Terrain JSON Snippet

```json
{
  "Objects": [
    {
      "Name": "terrain",
      "Definition": [
        {
          "Name": "Terrain",
          "File": "OBJ/sceneLeft_terrainLeft.obj",
          "Material": {
            "Type": "Blended",
            "Layers": [
              {
                "ColorMap": "Textures/aerial_grass_rock_diff_4k.jpg",
                "NormalMap": "Textures/aerial_grass_rock_nor_gl_4k.exr",
                "UVScale": 34
              },
              {
                "AlphaMap": "Masks/sceneLeft_terrainLeftmaskSlopeGround.png",
                "ColorMap": "Textures/Grass_Dry_BaseColor.jpg",
                "NormalMap": "Textures/aerial_rocks_04_nor_gl_2k.exr",
                "UVScale": 34
              },
              {
                "AlphaMap": "Masks/sceneLeft_terrainLeftmaskSlopeBedrock.png",
                "ColorMap": "Textures/rock_boulder_dry_diff_4k.jpg",
                "NormalMap": "Textures/rock_boulder_dry_nor_gl_4k.exr",
                "UVScale": 6.8
              }
            ]
          }
        }
      ]
    }
  ]
}
```

---

### 🧩 How It Works

* **Blended Material**
  The `"Type": "Blended"` value indicates that multiple texture layers are combined using **alpha masks**.
  Each mask defines which parts of the terrain each texture layer appears on (e.g., grass on flat areas, rock on steep slopes).

* **UVScale and Real-World Units**
  The `UVScale` parameter defines the **number of texture repetition on the terrain according to an extent in meter in the application** — how much real-world area a texture covers before repeating.

  * A smaller `UVScale` → texture repeats more often (finer detail).
  * A larger `UVScale` → texture stretches across larger distances (coarser detail).

  Example:

  * `UVScale: 34` → texture repeats roughly **34 times**.
  * `UVScale: 6.8` → texture repeats **6.8 times**.

* **Customizing the Terrain**
  You can adjust:

  1. **`ColorMap`** and **`NormalMap`** → change the surface textures.
  2. **`UVScale`** → control the texture density and repetition scale.
  3. **`AlphaMap`** → modify blending between layers.

These parameters can be edited in:

* The **terrain JSON file** exported by Ecoviz, or
* The **Ecoviz application source code**, where the default (hard-coded) values are defined.

---

### 💡 Tips for Realistic Results

* Use texture images of similar resolution (e.g., all 2K or 4K).
* Assign higher `UVScale` values to coarse materials (like rocks)
  and lower values to fine materials (like grass).
* Keep texture scales consistent with real-world proportions to maintain realism.

---

## 🧩 Configuring Mitsuba

If Mitsuba is not found automatically, set the `MITSUBA_DIR` environment variable to its installation path.

**Windows (PowerShell):**

```sh
setx MITSUBA_DIR "C:\mitsuba"
```

**macOS / Linux:**

```sh
export MITSUBA_DIR=/usr/local/mitsuba
```

You can also specify the rendering variant (CPU/GPU) in Python:

```python
import mitsuba
mitsuba.set_variant('scalar_rgb')     # CPU rendering
# or
mitsuba.set_variant('cuda_ad_rgb')    # GPU rendering (if supported)
```

---

## 🖼️ Output

Rendered images (e.g., `render.png` ) are saved to the current directory.
You can visualize them using any image viewer or load them in Python with `matplotlib` for analysis.

---

## 📚 References

* [Official Mitsuba Documentation](https://mitsuba-renderer.org/docs.html)
* [Physically-Based Rendering Concepts](https://www.pbrt.org/)

---

*Created by the Ecoviz Team — enabling physically-accurate scene rendering using Mitsuba and Python.*

```
