#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"

class material {
  public:
    virtual ~material() = default;

    virtual color emitted(double u, double v, const point3& p) const {
        return color(0,0,0);
    }

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const {
        return false;
    }
};

//Clase para manejar reflectancia difusa
class lambertian : public material {
  public:
    lambertian(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
    lambertian(shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        auto scatter_direction = rec.normal + random_unit_vector();

        // Identifica casos donde el vector aleatorio es igual al opuesto a la normal, asi evitando que regrese cero
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        //Crea rayo reflejado con direccion aleatoria
        scattered = ray(rec.p, scatter_direction, r_in.time());

        // Establece la atenuacion del color basado en el material
        attenuation = tex->value(rec.u, rec.v, rec.p);
        return true;
    }

  private:
    shared_ptr<texture> tex;
};

class metal : public material {
  public:
    metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    //Calcula el rebote de luz sobre una superficie metalica
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        //Para metales esto es un reflejo puntual en una direccion dada
        vec3 reflected = reflect(r_in.direction(), rec.normal);

        //Difumina el reflejo dependiendo del valor de "fuzz" del metal
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());

        //Crea y guarda el rayo reflejado con la direccion calculada
        scattered = ray(rec.p, reflected, r_in.time());

        //Determina cantidad de luz que refleja el metal
        attenuation = albedo;

        //Si el producto punto es mayor a 0, significa que el rayo se refleja hacia afuera de la superficie
        return (dot(scattered.direction(), rec.normal) > 0);
    }

  private:
    color albedo;
    double fuzz;
};

//Materialesa dialectricos que siempre refractan luz
class dielectric : public material {
  public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    //Para el rebote de un material dialectrico, se refracta utilizando la ley de Snell
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        attenuation = color(1.0, 1.0, 1.0);

        //Determina si la luz entra o sale del material para aplicar la ley de Snell correctamente
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());


        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        //Detecta angulos criticos donde no es posible aplicar la ley de Snell
        bool cannot_refract = ri * sin_theta > 1.0;

        vec3 direction;

        //Considera excepciones a la ley de Schnell
         if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        //Aplicacion de la ley de Snell
        else
            direction = refract(unit_direction, rec.normal, ri);

        scattered = ray(rec.p, direction, r_in.time());
        return true;
    }

  private:
    // Indice de refractacion del material dado
    double refraction_index;

    static double reflectance(double cosine, double refraction_index) {
        // Aproximacion Schlick para considerar variacion en reflectancia dependiendo del angulo
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1 - cosine),5);
    }
};

class diffuse_light : public material {
  public:
    diffuse_light(shared_ptr<texture> tex) : tex(tex) {}
    diffuse_light(const color& emit) : tex(make_shared<solid_color>(emit)) {}

    color emitted(double u, double v, const point3& p) const override {
        return tex->value(u, v, p);
    }

  private:
    shared_ptr<texture> tex;
};

class isotropic : public material {
  public:
    isotropic(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
    isotropic(shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        scattered = ray(rec.p, random_unit_vector(), r_in.time());
        attenuation = tex->value(rec.u, rec.v, rec.p);
        return true;
    }

  private:
    shared_ptr<texture> tex;
};

#endif