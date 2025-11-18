//Arreglar Earth, Quads, Simple Light, Cornell Box, Cornell Smoke

#include "GlassTracer.h"


#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"

#include "imgui.h"
#include <vector>
#include <memory>
#include <cstdio>
#include "material.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include "quad.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif

// A small struct to hold editable sphere parameters in the UI.
struct UISphere {
    float x, y, z;
    float radius;
    int material_type; // 0 = lambertian, 1 = metal, 2 = dielectric (optional)
    float albedo_r, albedo_g, albedo_b; // for lambert/metal
    float fuzz_or_ri; // fuzz for metal, refr_idx for dielectric
};

// UI struct for lights
struct UILight {
    float x, y, z;
    float radius;
    float r, g, b; // color
    float intensity; // multiplier for emission
};

// UI struct for quads
struct UIQuad {
    float corner_x, corner_y, corner_z;
    float sideA_x, sideA_y, sideA_z;
    float sideB_x, sideB_y, sideB_z;
    int material_type; // 0 = lambertian, 1 = metal, 2 = dielectric, 3 = diffuse_light
    float r, g, b;
    float fuzz_or_ri_or_intensity;
};

// Helper: Create material from UI parameters
static std::shared_ptr<material> create_material_from_ui(int mat_type, float r, float g, float b, float param) {
    if (mat_type == 1) return std::make_shared<metal>(color(r, g, b), param);
    if (mat_type == 2) return std::make_shared<dielectric>(param);
    if (mat_type == 3) return std::make_shared<diffuse_light>(color(r * param, g * param, b * param));
    return std::make_shared<lambertian>(color(r, g, b)); // default lambertian
}

// Load a preset scene into the UI data structures (camera + UI lists).
static void load_preset(int idx, camera& cam, std::vector<UISphere>& ui_spheres, std::vector<UILight>& ui_lights, std::vector<UIQuad>& ui_quads) {
    ui_spheres.clear();
    ui_lights.clear();
    ui_quads.clear();
    // default background for presets
    cam.background = color(0.70, 0.80, 1.00);

    switch (idx) {
    case 1: // bouncing_spheres (approximate)
        cam.aspect_ratio = 16.0/9.0;
        cam.image_width = 400;
        cam.samples_per_pixel = 100;
        cam.max_depth = 50;
        cam.vfov = 20; cam.lookfrom = point3(13,2,3); cam.lookat = point3(0,0,0);
        cam.defocus_angle = 0.6; cam.focus_dist = 10.0;
        // ground
        ui_spheres.push_back(UISphere{0.0f,-1000.0f,0.0f,1000.0f,0,0.5f,0.5f,0.5f,0.0f});
        // three example spheres
        ui_spheres.push_back(UISphere{0.0f,1.0f,0.0f,1.0f,2,0.95f,0.95f,0.95f,1.5f}); // glass
        ui_spheres.push_back(UISphere{-4.0f,1.0f,0.0f,1.0f,0,0.4f,0.2f,0.1f,0.0f});
        ui_spheres.push_back(UISphere{4.0f,1.0f,0.0f,1.0f,1,0.7f,0.6f,0.5f,0.0f});
        break;
    case 2: // checkered_spheres
        cam.background = color(0.70,0.80,1.00);
        cam.aspect_ratio = 16.0/9.0; 
        cam.image_width = 400; cam.samples_per_pixel = 100; cam.max_depth = 50;
        cam.vfov = 20; cam.lookfrom = point3(13,2,3); cam.lookat = point3(0,0,0); cam.defocus_angle = 0;
        ui_spheres.push_back(UISphere{0.0f,-10.0f,0.0f,10.0f,0,0.2f,0.3f,0.1f,0.0f});
        ui_spheres.push_back(UISphere{0.0f,10.0f,0.0f,10.0f,0,0.9f,0.9f,0.9f,0.0f});
        break;
    case 3: // earth (approximate as lambertian)
        cam.background = color(0.70,0.80,1.00);
        cam.aspect_ratio = 16.0/9.0; cam.image_width = 400; cam.samples_per_pixel = 100; cam.max_depth = 50;
        cam.vfov = 20; cam.lookfrom = point3(0,0,12); cam.lookat = point3(0,0,0); cam.defocus_angle = 0;
        ui_spheres.push_back(UISphere{0.0f,0.0f,0.0f,2.0f,0,0.6f,0.6f,0.6f,0.0f});
        break;
    case 4: // perlin_spheres
        cam.background = color(0.70,0.80,1.00);
        cam.aspect_ratio = 16.0/9.0; cam.image_width = 400; cam.samples_per_pixel = 100; cam.max_depth = 50;
        cam.vfov = 20; cam.lookfrom = point3(13,2,3); cam.lookat = point3(0,0,0); cam.defocus_angle = 0;
        ui_spheres.push_back(UISphere{0.0f,-1000.0f,0.0f,1000.0f,0,0.5f,0.5f,0.5f,0.0f});
        ui_spheres.push_back(UISphere{0.0f,2.0f,0.0f,2.0f,0,0.6f,0.6f,0.6f,0.0f});
        break;
    case 5: // quads
        cam.background = color(0.70,0.80,1.00);
        cam.aspect_ratio = 1.0; cam.image_width = 400; cam.samples_per_pixel = 100; cam.max_depth = 50;
        cam.vfov = 80; cam.lookfrom = point3(0,0,9); cam.lookat = point3(0,0,0); cam.defocus_angle = 0;
        // Materials: left_red, back_green, right_blue, upper_orange, lower_teal
        ui_quads.push_back(UIQuad{-3.0f,-2.0f,5.0f, 0.0f,0.0f,-4.0f, 0.0f,4.0f,0.0f, 0, 1.0f,0.2f,0.2f, 0.0f}); // left_red
        ui_quads.push_back(UIQuad{-2.0f,-2.0f,0.0f, 4.0f,0.0f,0.0f, 0.0f,4.0f,0.0f, 0, 0.2f,1.0f,0.2f, 0.0f}); // back_green
        ui_quads.push_back(UIQuad{3.0f,-2.0f,1.0f, 0.0f,0.0f,4.0f, 0.0f,4.0f,0.0f, 0, 0.2f,0.2f,1.0f, 0.0f}); // right_blue
        ui_quads.push_back(UIQuad{-2.0f,3.0f,1.0f, 4.0f,0.0f,0.0f, 0.0f,0.0f,4.0f, 0, 1.0f,0.5f,0.0f, 0.0f}); // upper_orange
        ui_quads.push_back(UIQuad{-2.0f,-3.0f,5.0f, 4.0f,0.0f,0.0f, 0.0f,0.0f,-4.0f, 0, 0.2f,0.8f,0.8f, 0.0f}); // lower_teal
        break;
    case 6: // simple_light
        cam.aspect_ratio = 16.0/9.0; cam.image_width = 400; cam.samples_per_pixel = 100; cam.max_depth = 50; cam.background = color(0,0,0);
        cam.vfov = 20; cam.lookfrom = point3(26,3,6); cam.lookat = point3(0,2,0); cam.defocus_angle = 0;
        ui_spheres.push_back(UISphere{0.0f,-1000.0f,0.0f,1000.0f,0,0.5f,0.5f,0.5f,0.0f});
        ui_spheres.push_back(UISphere{0.0f,2.0f,0.0f,2.0f,0,0.6f,0.6f,0.6f,0.0f});
        // Add an emissive sphere light (as in reference)
        ui_lights.push_back(UILight{0.0f,7.0f,0.0f,2.0f,1.0f,1.0f,1.0f,4.0f});
        // Add an emissive quad (area light)
        // quad corner (3,1,-2), sideA (2,0,0), sideB (0,2,0), diffuse_light color (4,4,4)
        // We store color as base (1,1,1) and intensity as 4.0 so create_material_from_ui will create (4,4,4)
        ui_quads.push_back(UIQuad{3.0f,1.0f,-2.0f, 2.0f,0.0f,0.0f, 0.0f,2.0f,0.0f, 3, 1.0f,1.0f,1.0f, 4.0f});
        break;
    case 7: // cornell_box (approximate)
        cam.aspect_ratio = 1.0; cam.image_width = 600; cam.samples_per_pixel = 200; cam.max_depth = 50; cam.background = color(0,0,0);
        cam.vfov = 40; cam.lookfrom = point3(278,278,-800); cam.lookat = point3(278,278,0); cam.defocus_angle = 0;
        // Cornell box walls (use lambertian materials)
        // left (green)
        ui_quads.push_back(UIQuad{555.0f,0.0f,0.0f, 0.0f,555.0f,0.0f, 0.0f,0.0f,555.0f, 0, 0.12f,0.45f,0.15f, 0.0f});
        // right (red)
        ui_quads.push_back(UIQuad{0.0f,0.0f,0.0f, 0.0f,555.0f,0.0f, 0.0f,0.0f,555.0f, 0, 0.65f,0.05f,0.05f, 0.0f});
        // light (small quad on ceiling)
        // store base color (1,1,1) with intensity 15
        ui_quads.push_back(UIQuad{343.0f,554.0f,332.0f, -130.0f,0.0f,0.0f, 0.0f,0.0f,-105.0f, 3, 1.0f,1.0f,1.0f, 15.0f});
        // other walls - white
        ui_quads.push_back(UIQuad{0.0f,0.0f,0.0f, 555.0f,0.0f,0.0f, 0.0f,0.0f,555.0f, 0, 0.73f,0.73f,0.73f, 0.0f});
        ui_quads.push_back(UIQuad{555.0f,555.0f,555.0f, -555.0f,0.0f,0.0f, 0.0f,0.0f,-555.0f, 0, 0.73f,0.73f,0.73f, 0.0f});
        ui_quads.push_back(UIQuad{0.0f,0.0f,555.0f, 555.0f,0.0f,0.0f, 0.0f,555.0f,0.0f, 0, 0.73f,0.73f,0.73f, 0.0f});
        break;
    case 8: // cornell_smoke (approximate)
        cam.aspect_ratio = 1.0; cam.image_width = 600; cam.samples_per_pixel = 200; cam.max_depth = 50; cam.background = color(0,0,0);
        cam.vfov = 40; cam.lookfrom = point3(278,278,-800); cam.lookat = point3(278,278,0); cam.defocus_angle = 0;
        // Similar to cornell_box but with a dimmer light
        ui_quads.push_back(UIQuad{555.0f,0.0f,0.0f, 0.0f,555.0f,0.0f, 0.0f,0.0f,555.0f, 0, 0.12f,0.45f,0.15f, 0.0f});
        ui_quads.push_back(UIQuad{0.0f,0.0f,0.0f, 0.0f,555.0f,0.0f, 0.0f,0.0f,555.0f, 0, 0.65f,0.05f,0.05f, 0.0f});
        ui_quads.push_back(UIQuad{113.0f,554.0f,127.0f, 330.0f,0.0f,0.0f, 0.0f,0.0f,305.0f, 3, 1.0f,1.0f,1.0f, 7.0f});
        ui_quads.push_back(UIQuad{0.0f,555.0f,0.0f, 555.0f,0.0f,0.0f, 0.0f,0.0f,555.0f, 0, 0.73f,0.73f,0.73f, 0.0f});
        ui_quads.push_back(UIQuad{0.0f,0.0f,0.0f, 555.0f,0.0f,0.0f, 0.0f,0.0f,555.0f, 0, 0.73f,0.73f,0.73f, 0.0f});
        ui_quads.push_back(UIQuad{0.0f,0.0f,555.0f, 555.0f,0.0f,0.0f, 0.0f,555.0f,0.0f, 0, 0.73f,0.73f,0.73f, 0.0f});
        break;
    default:
        break;
    }
}

static void show_scene_editor(hittable_list& world, camera& cam, std::vector<UISphere>& ui_spheres, std::vector<UILight>& ui_lights, std::vector<UIQuad>& ui_quads, bool& request_render) {
    ImGui::Begin("Scene Editor");

    // Preset scenes selection
    static const char* preset_names[] = {
        "(none)",
        "1 - Bouncing spheres",
        "2 - Checkered spheres",
        "3 - Earth",
        "4 - Perlin spheres",
        "5 - Quads",
        "6 - Simple light",
        "7 - Cornell box",
        "8 - Cornell smoke"
    };
    static int preset_idx = 0;
    ImGui::Combo("Presets", &preset_idx, preset_names, IM_ARRAYSIZE(preset_names));
    ImGui::SameLine();
    if (ImGui::Button("Load Preset")) {
        if (preset_idx > 0) {
            load_preset(preset_idx, cam, ui_spheres, ui_lights, ui_quads);
        }
    }

    // Add new sphere controls
    if (ImGui::CollapsingHeader("Add new sphere", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushID("new_sphere");
        // Field order: x,y,z, radius, material_type, albedo_r,g,b, fuzz_or_ri
        static UISphere new_sphere = {0.0f, 0.0f, -1.0f, 0.5f, 0, 0.7f, 0.6f, 0.5f, 0.0f};
        ImGui::InputFloat3("Position", &new_sphere.x);
        ImGui::InputFloat("Radius", &new_sphere.radius);
        ImGui::ColorEdit3("Albedo", &new_sphere.albedo_r);
        ImGui::Combo("Material", &new_sphere.material_type, "Lambertian\0Metal\0Dielectric\0");
        if (new_sphere.material_type == 1) ImGui::InputFloat("Fuzz", &new_sphere.fuzz_or_ri);
        if (new_sphere.material_type == 2) ImGui::InputFloat("Refraction idx", &new_sphere.fuzz_or_ri);

        if (ImGui::Button("Add Sphere")) {
            ui_spheres.push_back(new_sphere);
        }
        ImGui::PopID();
    }

    // Lights editor
    if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushID("new_light");
        static UILight new_light = {0.0f, 1.0f, -1.0f, 0.2f, 1.0f, 1.0f, 1.0f, 4.0f};
        ImGui::InputFloat3("Position", &new_light.x);
        ImGui::InputFloat("Radius", &new_light.radius);
        ImGui::ColorEdit3("Color", &new_light.r);
        ImGui::InputFloat("Intensity", &new_light.intensity);
        if (ImGui::Button("Add Light")) {
            ui_lights.push_back(new_light);
        }
        ImGui::PopID();

        ImGui::Separator();
        ImGui::Text("Lights in scene: %d", (int)ui_lights.size());
        for (int i = 0; i < (int)ui_lights.size(); ++i) {
            ImGui::PushID(i);
            char label[64];
            sprintf_s(label, sizeof(label), "Light %d", i);
            if (ImGui::TreeNode(label)) {
                ImGui::InputFloat3("Position", &ui_lights[i].x);
                ImGui::InputFloat("Radius", &ui_lights[i].radius);
                ImGui::ColorEdit3("Color", &ui_lights[i].r);
                ImGui::InputFloat("Intensity", &ui_lights[i].intensity);
                if (ImGui::Button("Remove Light")) {
                    ui_lights.erase(ui_lights.begin() + i);
                    ImGui::TreePop();
                    ImGui::PopID();
                    break;
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }

    // Add new quad controls
    if (ImGui::CollapsingHeader("Add new quad", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushID("new_quad");
        static UIQuad new_quad = {0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0, 0.7f, 0.6f, 0.5f, 0.0f};
        ImGui::InputFloat3("Corner##quad", &new_quad.corner_x);
        ImGui::InputFloat3("Side A##quad", &new_quad.sideA_x);
        ImGui::InputFloat3("Side B##quad", &new_quad.sideB_x);
        ImGui::ColorEdit3("Color##quad", &new_quad.r);
        ImGui::Combo("Material##quad", &new_quad.material_type, "Lambertian\0Metal\0Dielectric\0Diffuse Light\0");
        if (new_quad.material_type == 1) ImGui::InputFloat("Fuzz##quad", &new_quad.fuzz_or_ri_or_intensity);
        if (new_quad.material_type == 2) ImGui::InputFloat("Refraction idx##quad", &new_quad.fuzz_or_ri_or_intensity);
        if (new_quad.material_type == 3) ImGui::InputFloat("Intensity##quad", &new_quad.fuzz_or_ri_or_intensity);

        if (ImGui::Button("Add Quad")) {
            ui_quads.push_back(new_quad);
        }
        ImGui::PopID();
    }

    // List & edit existing quads
    if (ui_quads.size() > 0) {
        ImGui::Separator();
        ImGui::Text("Quads in scene: %d", (int)ui_quads.size());
        for (int i = 0; i < (int)ui_quads.size(); ++i) {
            char label[64];
            sprintf_s(label, sizeof(label), "Quad %d", i);
            ImGui::PushID(100 + i);  // offset to avoid conflicts with sphere indices
            if (ImGui::TreeNode(label)) {
                ImGui::InputFloat3("Corner", &ui_quads[i].corner_x);
                ImGui::InputFloat3("Side A", &ui_quads[i].sideA_x);
                ImGui::InputFloat3("Side B", &ui_quads[i].sideB_x);
                ImGui::ColorEdit3("Color", &ui_quads[i].r);
                ImGui::Combo("Material", &ui_quads[i].material_type, "Lambertian\0Metal\0Dielectric\0Diffuse Light\0");
                if (ui_quads[i].material_type == 1) ImGui::InputFloat("Fuzz", &ui_quads[i].fuzz_or_ri_or_intensity);
                if (ui_quads[i].material_type == 2) ImGui::InputFloat("Refraction idx", &ui_quads[i].fuzz_or_ri_or_intensity);
                if (ui_quads[i].material_type == 3) ImGui::InputFloat("Intensity", &ui_quads[i].fuzz_or_ri_or_intensity);

                if (ImGui::Button("Remove")) {
                    ui_quads.erase(ui_quads.begin() + i);
                    ImGui::TreePop();
                    ImGui::PopID();
                    break;
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }
    ImGui::Separator();
    ImGui::Text("Spheres in scene: %d", (int)ui_spheres.size());
    for (int i = 0; i < (int)ui_spheres.size(); ++i) {
        char label[64];
        sprintf_s(label, sizeof(label), "Sphere %d", i);
        ImGui::PushID(i);
        if (ImGui::TreeNode(label)) {
            ImGui::InputFloat3("Position", &ui_spheres[i].x);
            ImGui::InputFloat("Radius", &ui_spheres[i].radius);
            ImGui::ColorEdit3("Albedo", &ui_spheres[i].albedo_r);
            ImGui::Combo("Material", &ui_spheres[i].material_type, "Lambertian\0Metal\0Dielectric\0");
            if (ui_spheres[i].material_type == 1) ImGui::InputFloat("Fuzz", &ui_spheres[i].fuzz_or_ri);
            if (ui_spheres[i].material_type == 2) ImGui::InputFloat("Refraction idx", &ui_spheres[i].fuzz_or_ri);

            if (ImGui::Button("Remove")) {
                ui_spheres.erase(ui_spheres.begin() + i);
                ImGui::TreePop();
                ImGui::PopID();
                break; // indices shifted, restart loop next frame
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    ImGui::Separator();

    // Camera controls (optional)
    if (ImGui::CollapsingHeader("Camera")) {
        // Use temporary float buffers to avoid feeding float pointers into double-backed vectors
        static float vfov_f = (float)cam.vfov;
        static float lookfrom_f[3] = { (float)cam.lookfrom[0], (float)cam.lookfrom[1], (float)cam.lookfrom[2] };
        static float lookat_f[3]   = { (float)cam.lookat[0],   (float)cam.lookat[1],   (float)cam.lookat[2] };

        ImGui::InputFloat("vfov", &vfov_f);
        ImGui::InputInt("Image width", &cam.image_width);
        ImGui::InputInt("Samples per pixel", &cam.samples_per_pixel);
        ImGui::InputFloat3("Lookfrom", lookfrom_f);
        ImGui::InputFloat3("Lookat", lookat_f);

        // Apply camera UI edits back into the camera when changed (keeps types consistent)
        cam.vfov = (double)vfov_f;
        cam.lookfrom = point3((double)lookfrom_f[0], (double)lookfrom_f[1], (double)lookfrom_f[2]);
        cam.lookat   = point3((double)lookat_f[0],   (double)lookat_f[1],   (double)lookat_f[2]);
    }

    // Buttons
    if (ImGui::Button("Apply to world")) {
        // Rebuild world from ui_spheres and ui_lights, write a scene file for headless rendering.
        world.clear();
        for (auto &s : ui_spheres) {
            std::shared_ptr<material> mat = std::make_shared<lambertian>(color(s.albedo_r, s.albedo_g, s.albedo_b));
            if (s.material_type == 1) mat = std::make_shared<metal>(color(s.albedo_r, s.albedo_g, s.albedo_b), s.fuzz_or_ri);
            if (s.material_type == 2) mat = std::make_shared<dielectric>(s.fuzz_or_ri);
            world.add(std::make_shared<sphere>(point3(s.x, s.y, s.z), s.radius, mat));
        }

        // Add lights to the world as small emissive spheres
        for (auto &L : ui_lights) {
            color emit = color(L.r * L.intensity, L.g * L.intensity, L.b * L.intensity);
            std::shared_ptr<material> lm = std::make_shared<diffuse_light>(emit);
            world.add(std::make_shared<sphere>(point3(L.x, L.y, L.z), L.radius, lm));
        }

        // Add quads to the world
        for (auto &q : ui_quads) {
            std::shared_ptr<material> mat = create_material_from_ui(q.material_type, q.r, q.g, q.b, q.fuzz_or_ri_or_intensity);
            world.add(std::make_shared<quad>(point3(q.corner_x, q.corner_y, q.corner_z),
                                            vec3(q.sideA_x, q.sideA_y, q.sideA_z),
                                            vec3(q.sideB_x, q.sideB_y, q.sideB_z),
                                            mat));
        }

        // Serialize simple scene file next to the executable (scene.txt)
        std::string exeFolder = ".";
#ifdef _WIN32
        char modulePath[MAX_PATH];
        GetModuleFileNameA(NULL, modulePath, MAX_PATH);
        PathRemoveFileSpecA(modulePath);
        exeFolder = std::string(modulePath);
#endif

        std::string scenePath = exeFolder + "\\scene.txt";
        std::ofstream out(scenePath);
        if (out) {
            // Camera header line
            out << cam.image_width << " " << cam.samples_per_pixel << " " << cam.vfov << " " << cam.aspect_ratio << " "
                << cam.defocus_angle << " " << cam.focus_dist << " "
                << cam.lookfrom[0] << " " << cam.lookfrom[1] << " " << cam.lookfrom[2] << " "
                << cam.lookat[0] << " " << cam.lookat[1] << " " << cam.lookat[2] << " "
                << cam.background.x() << " " << cam.background.y() << " " << cam.background.z() << "\n";

            // Spheres
            out << ui_spheres.size() << "\n";
            for (auto &s : ui_spheres) {
                out << s.x << " " << s.y << " " << s.z << " " << s.radius << " " << s.material_type << " "
                    << s.albedo_r << " " << s.albedo_g << " " << s.albedo_b << " " << s.fuzz_or_ri << "\n";
            }

            // Lights
            out << ui_lights.size() << "\n";
            for (auto &L : ui_lights) {
                out << L.x << " " << L.y << " " << L.z << " " << L.radius << " "
                    << L.r << " " << L.g << " " << L.b << " " << L.intensity << "\n";
            }

            // Quads
            out << ui_quads.size() << "\n";
            for (auto &q : ui_quads) {
                out << q.corner_x << " " << q.corner_y << " " << q.corner_z << " "
                    << q.sideA_x << " " << q.sideA_y << " " << q.sideA_z << " "
                    << q.sideB_x << " " << q.sideB_y << " " << q.sideB_z << " "
                    << q.material_type << " " << q.r << " " << q.g << " " << q.b << " " << q.fuzz_or_ri_or_intensity << "\n";
            }

            out.close();
        }

        // Signal caller (main) to start headless render using the scene file.
        request_render = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Render (blocking)")) {
        // 1) Apply changes, 2) request render
        world.clear();
        for (auto &s : ui_spheres) {
            std::shared_ptr<material> mat = std::make_shared<lambertian>(color(s.albedo_r, s.albedo_g, s.albedo_b));
            if (s.material_type == 1) mat = std::make_shared<metal>(color(s.albedo_r, s.albedo_g, s.albedo_b), s.fuzz_or_ri);
            if (s.material_type == 2) mat = std::make_shared<dielectric>(s.fuzz_or_ri);
            world.add(std::make_shared<sphere>(point3(s.x, s.y, s.z), s.radius, mat));
        }
        // Add quads
        for (auto &q : ui_quads) {
            std::shared_ptr<material> mat = create_material_from_ui(q.material_type, q.r, q.g, q.b, q.fuzz_or_ri_or_intensity);
            world.add(std::make_shared<quad>(point3(q.corner_x, q.corner_y, q.corner_z),
                                            vec3(q.sideA_x, q.sideA_y, q.sideA_z),
                                            vec3(q.sideB_x, q.sideB_y, q.sideB_z),
                                            mat));
        }
        // Set this flag for the main loop to call cam.render(world).
        request_render = true;
    }

    ImGui::End();
}

int main(int argc, char* argv[]) {
    // Headless render mode: program launched with --render <scenefile>
    if (argc >= 3 && std::string(argv[1]) == "--render") {
        std::string scene_file = argv[2];
        std::ifstream in(scene_file);
        if (!in) {
            std::cerr << "Failed to open scene file: " << scene_file << "\n";
            return -1;
        }

        camera cam;
        hittable_list world;

        int image_w = 0, samples = 0;
        double vfov = 0, aspect = 1.0, defocus = 0, focus = 10.0;
        double lf_x=0, lf_y=0, lf_z=0, la_x=0, la_y=0, la_z= -1;
        double bg_r=0, bg_g=0, bg_b=0;
        in >> image_w >> samples >> vfov >> aspect >> defocus >> focus
           >> lf_x >> lf_y >> lf_z >> la_x >> la_y >> la_z
           >> bg_r >> bg_g >> bg_b;

        cam.image_width = image_w;
        cam.samples_per_pixel = samples;
        cam.vfov = vfov;
        cam.aspect_ratio = aspect;
        cam.defocus_angle = defocus;
        cam.focus_dist = focus;
        cam.lookfrom = point3(lf_x, lf_y, lf_z);
        cam.lookat = point3(la_x, la_y, la_z);
        cam.background = color(bg_r, bg_g, bg_b);

        int sphere_count = 0;
        in >> sphere_count;
        for (int s = 0; s < sphere_count; ++s) {
            double x,y,z,radius; int mat_type; double ar,ag,ab; double f_or_ri;
            in >> x >> y >> z >> radius >> mat_type >> ar >> ag >> ab >> f_or_ri;
            std::shared_ptr<material> mat = std::make_shared<lambertian>(color(ar,ag,ab));
            if (mat_type == 1) mat = std::make_shared<metal>(color(ar,ag,ab), f_or_ri);
            if (mat_type == 2) mat = std::make_shared<dielectric>(f_or_ri);
            world.add(std::make_shared<sphere>(point3(x,y,z), radius, mat));
        }

        int light_count = 0;
        in >> light_count;
        for (int l = 0; l < light_count; ++l) {
            double x,y,z,radius, r,g,b, intensity;
            in >> x >> y >> z >> radius >> r >> g >> b >> intensity;
            color emit = color(r * intensity, g * intensity, b * intensity);
            std::shared_ptr<material> mat = std::make_shared<diffuse_light>(emit);
            world.add(std::make_shared<sphere>(point3(x,y,z), radius, mat));
        }

        int quad_count = 0;
        in >> quad_count;
        for (int q = 0; q < quad_count; ++q) {
            double cx, cy, cz, sax, say, saz, sbx, sby, sbz;
            int mat_type;
            double r, g, b, param;
            in >> cx >> cy >> cz >> sax >> say >> saz >> sbx >> sby >> sbz >> mat_type >> r >> g >> b >> param;
            std::shared_ptr<material> mat = create_material_from_ui(mat_type, (float)r, (float)g, (float)b, (float)param);
            world.add(std::make_shared<quad>(point3(cx, cy, cz), vec3(sax, say, saz), vec3(sbx, sby, sbz), mat));
        }

        // Set working directory to exe folder is handled by parent process when launching CreateProcess with lpCurrentDirectory
        cam.render(world);
        return 0;
    }
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // GL 3.3 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Glass Tracer", nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup scene
    hittable_list world;
    camera cam;
    std::vector<UISphere> ui_spheres;
    std::vector<UILight> ui_lights;
    std::vector<UIQuad> ui_quads;
    bool request_render = false;

    // Add a default light above the camera if none exist
    if (ui_lights.empty()) {
        UILight default_light;
        default_light.x = (float)cam.lookfrom[0];
        default_light.y = (float)cam.lookfrom[1] + 2.0f; // above the camera
        default_light.z = (float)cam.lookfrom[2];
        default_light.radius = 0.2f;
        default_light.r = 1.0f; default_light.g = 1.0f; default_light.b = 1.0f;
        default_light.intensity = 4.0f;
        ui_lights.push_back(default_light);
    }

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

    // Show the scene editor
    show_scene_editor(world, cam, ui_spheres, ui_lights, ui_quads, request_render);

        // Handle render request: if set by Apply button, close GUI and launch headless renderer
        if (request_render) {
            // Determine exe folder and scene file path
            std::string exeFolder = ".";
#ifdef _WIN32
            char modulePath[MAX_PATH];
            GetModuleFileNameA(NULL, modulePath, MAX_PATH);
            // Module path contains full path to exe; strip exe name to get folder
            PathRemoveFileSpecA(modulePath);
            exeFolder = std::string(modulePath);
#endif
            std::string scenePath = exeFolder + "\\scene.txt";

            // Cleanup GUI resources
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            glfwDestroyWindow(window);
            glfwTerminate();

#ifdef _WIN32
            // Launch a new process in a new console running this exe in headless mode
            char exeFull[MAX_PATH];
            GetModuleFileNameA(NULL, exeFull, MAX_PATH);
            std::string exeFullPath = std::string(exeFull);
            std::string cmdLine = "\"" + exeFullPath + "\" --render \"" + scenePath + "\"";

            STARTUPINFOA si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            BOOL ok = CreateProcessA(
                exeFullPath.c_str(),
                (LPSTR)cmdLine.c_str(),
                NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
                NULL,
                exeFolder.c_str(),
                &si, &pi);

            if (ok) {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
            } else {
                std::cerr << "Failed to launch headless render process.\n";
            }

            return 0;
#else
            std::cout << "Scene saved to: " << scenePath << "\n";
            std::cout << "Please run: <exe> --render " << scenePath << " from a console in " << exeFolder << "\n";
            return 0;
#endif
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}