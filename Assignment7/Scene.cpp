//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"

void Scene::buildBVH() {
  printf(" - Generating BVH...\n\n");
  this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const {
  return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const {
  float emit_area_sum = 0;
  for (uint32_t k = 0; k < objects.size(); ++k) {
    if (objects[k]->hasEmit()) {
      emit_area_sum += objects[k]->getArea();
    }
  }
  float p = get_random_float() * emit_area_sum;
  emit_area_sum = 0;
  for (uint32_t k = 0; k < objects.size(); ++k) {
    if (objects[k]->hasEmit()) {
      emit_area_sum += objects[k]->getArea();
      if (p <= emit_area_sum) {
        objects[k]->Sample(pos, pdf);
        break;
      }
    }
  }
}

bool Scene::trace(const Ray &ray, const std::vector<Object *> &objects,
                  float &tNear, uint32_t &index, Object **hitObject) {
  *hitObject = nullptr;
  for (uint32_t k = 0; k < objects.size(); ++k) {
    float tNearK = kInfinity;
    uint32_t indexK;
    Vector2f uvK;
    if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
      *hitObject = objects[k];
      tNear = tNearK;
      index = indexK;
    }
  }

  return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const {
  // TO DO Implement Path Tracing Algorithm here

  // 找到着色点
  Intersection P = intersect(ray);
  if (!P.happened) {
    return Vector3f(0, 0, 0);
  }
  if (P.m->hasEmission()) {
    return P.m->getEmission();
  }

  // 直接光照
  Intersection light;
  float pdf;
  sampleLight(light, pdf);

  Vector3f wo = (ray.direction - P.coords).normalized();
  // Vector3f wo = ray.direction;

  Vector3f wi = (light.coords - P.coords).normalized();

  // 判断直接光源是否被遮挡
  Ray tmp(P.coords, wi);
  Intersection P2 = intersect(tmp);
  double eps = 1e-6;
  Vector3f L(0, 0, 0);
  if (P2.distance - (light.coords - P.coords).norm() > -0.005) {
    // 计算着色点fr
    Vector3f fr = P.m->eval(wo, wi, P.normal);
    // 渲染方程 Lo = Li(x,wi) * fr * cos\theta * cos\theta' / r2 / pdf
    float r2 = dotProduct(light.coords - P.coords, light.coords - P.coords);
    L += light.emit * fr * dotProduct(wi, P.normal) * dotProduct(light.normal, -wi) / r2 / pdf;
  }

  // 间接光照
  if (get_random_float() > RussianRoulette) {
    return L;
  }

  // 随机采样入射方向
  wi = P.m->sample(wo, P.normal).normalized();
  Ray tmp2(P.coords, wi);
  Intersection P3 = intersect(tmp2);

  // 确实有物体
  if (P3.happened && !P3.m->hasEmission()) {
    float pdf = P.m->pdf(wo, wi, P.normal);
    Vector3f fr = P.m->eval(wo, wi, P.normal);
    // 渲染方程 Lo = Li(x,wi) * fr * cos\theta  / pdf / RussianRoulette
    L += castRay(tmp2, depth + 1) * fr * dotProduct(wi, P.normal) * dotProduct(wi, P.normal) / pdf/ RussianRoulette;
  }

  return L;
}