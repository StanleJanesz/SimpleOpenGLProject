# 🌌 3D Scene Simulation

A real-time 3D graphics application showcasing a dynamic scene composed of various geometric objects, multiple cameras, and advanced lighting effects. Built to demonstrate key techniques in real-time rendering including deferred shading, fog, and dynamic lighting transitions.


https://github.com/user-attachments/assets/d3d8ad31-c91d-49d2-a858-3bd46d8d0232




## 🧩 Features

### 🎮 Scene Elements

- ✅ A simple 3D scene consisting of:
  - Moving objects (e.g., rotating and translating spheres, cubes)
  - Static objects, including:
    - At least one **smooth-shaded sphere**
    - Other fixed 3D primitives


### 📷 Camera System

Supports at least **three switchable cameras**:

1. **Static Camera** — observing the entire scene
2. **Tracking Camera** — follows the motion of a dynamic object
3. **Attached Camera (FPP/TPP)** — moves with a dynamic object  
   (_e.g., First or Third Person View_)

Switching between cameras is possible in real-time.

---

### 💡 Lighting System

At least **three light sources**, including:

- 🔦 **Spotlight** on a moving object (e.g., car headlights)
  - Adjustable spotlight direction relative to the moving object
- 💡 **Static light source**
  - Can be either **point light** or **spotlight**
- 🔥 **Dynamic attenuation** — lights fade with distance

---

### 🎨 Visual Effects & Rendering

- 🔭 **Perspective projection**
- 🧪 **Phong shading model**
  - Normal vector interpolation for smooth lighting
- 🌫️ **Fog** with smooth transitions
- 🌗 **Day/Night mode toggle**
- ✨ **Specular model toggle:** Choose between **Phong** and **Blinn-Phong**
- 📸 **Lighting calculated in camera space**
- 🧱 **Deferred shading** implemented for performance and realism

---

## 🛠️ Technologies Used

- OpenGL 
- GLSL shaders
- C++




