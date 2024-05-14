#include "material.h"
#include "pbr.h"
#include <time.h>
#include "mat4.h"
#include "sphere.h"
#include "random.h"

//------------------------------------------------------------------------------
/**
*/
Ray
BSDF(Material const* const material, Ray ray, vec3 point, vec3 normal)
{
    vec3 rayDir = ray.dir;
    float cosTheta = -dot(normalize(rayDir), normalize(normal));
    float r = RandomFloat();

    if (material->type != "Dielectric")
    {
        // Use this terr thing
        float F0 = (material->type == "Conductor") ? F0 = 0.95f : F0 = 0.04f;

        // probability that a ray will reflect on a microfacet
        float F = FresnelSchlick(cosTheta, F0, material->roughness);

        // Move up
        //float r = RandomFloat();

        if (r < F)
        {
            mat4 basis = TBN(normal);
            // importance sample with brdf specular lobe
            vec3 H = ImportanceSampleGGX_VNDF(RandomFloat(), RandomFloat(), material->roughness, rayDir, basis);
            vec3 reflected = reflect(rayDir, H);
            return { point, normalize(reflected) };
        }
        else
        {
            return { point, normalize(normalize(normal) + random_point_on_unit_sphere()) };
        }
    }
    else
    {
        // Added another ter thing
        vec3 outwardNormal = (cosTheta <= 0) ? -normal : normal;
        float niOverNt = (cosTheta <= 0) ? material->refractionIndex : 1.0f/material->refractionIndex;
        float cosine = (cosTheta <= 0) ? cosTheta * niOverNt / len(rayDir) : cosTheta / len(rayDir);

        vec3 refracted;

		// fresnel reflectance at 0 deg incidence angle
		float F0 = powf(material->refractionIndex - 1, 2) / powf(material->refractionIndex + 1, 2);
		float reflect_prob = FresnelSchlick(cosine, F0, material->roughness);

        // Remove all this
        //if (cosTheta <= 0)
        //{
        //    outwardNormal = -normal;
        //    niOverNt = material->refractionIndex;
        //    cosine = cosTheta * niOverNt / len(rayDir);
        //}
        //else
        //{
        //    outwardNormal = normal;
        //    niOverNt = 1.0 / material->refractionIndex;
        //    cosine = cosTheta / len(rayDir);
        //}

        if (Refract(normalize(rayDir), outwardNormal, niOverNt, refracted))
        {
            // Add this inside if statement
			if (r < reflect_prob)
			{
				vec3 reflected = reflect(rayDir, normal);
				return { point, reflected };
			}
			else
			{
				return { point, refracted };
			}
        }
        else
        {
            vec3 reflected = reflect(rayDir, normal);
            return { point, refracted };
        }
    }
}

//Ray
//BSDF(Material const* const material, Ray ray, vec3 point, vec3 normal)
//{
//    float cosTheta = -dot(normalize(ray.dir), normalize(normal));
//    float r = RandomFloat();
//
//    if (material->type != "Dielectric")
//    {
//        float F0 = 0.04f;
//        if (material->type == "Conductor")
//        {
//            F0 = 0.95f;
//        }
//
//        // probability that a ray will reflect on a microfacet
//        float F = FresnelSchlick(cosTheta, F0, material->roughness);
//
//        float r = RandomFloat();
//
//        if (r < F)
//        {
//            mat4 basis = TBN(normal);
//            // importance sample with brdf specular lobe
//            vec3 H = ImportanceSampleGGX_VNDF(RandomFloat(), RandomFloat(), material->roughness, ray.dir, basis);
//            vec3 reflected = reflect(ray.dir, H);
//            return { point, normalize(reflected) };
//        }
//        else
//        {
//            return { point, normalize(normalize(normal) + random_point_on_unit_sphere()) };
//        }
//    }
//    else
//    {
//        vec3 outwardNormal;
//        float niOverNt;
//        vec3 refracted;
//        float reflect_prob;
//        float cosine;
//        vec3 rayDir = ray.dir;
//
//        if (cosTheta <= 0)
//        {
//            outwardNormal = -normal;
//            niOverNt = material->refractionIndex;
//            cosine = cosTheta * niOverNt / len(rayDir);
//        }
//        else
//        {
//            outwardNormal = normal;
//            niOverNt = 1.0 / material->refractionIndex;
//            cosine = cosTheta / len(rayDir);
//        }
//
//        if (Refract(normalize(rayDir), outwardNormal, niOverNt, refracted))
//        {
//            // fresnel reflectance at 0 deg incidence angle
//            float F0 = powf(material->refractionIndex - 1, 2) / powf(material->refractionIndex + 1, 2);
//            reflect_prob = FresnelSchlick(cosine, F0, material->roughness);
//        }
//        else
//        {
//            reflect_prob = 1.0;
//        }
//        if (RandomFloat() < reflect_prob)
//        {
//            vec3 reflected = reflect(rayDir, normal);
//            return { point, reflected };
//        }
//        else
//        {
//            return { point, refracted };
//        }
//    }
//}