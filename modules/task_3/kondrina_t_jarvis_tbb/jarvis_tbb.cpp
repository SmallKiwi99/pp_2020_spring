// Copyright 2020 Kondrina Tatyana

#include "../../../modules/task_3/kondrina_t_jarvis_tbb/jarvis_tbb.h"

#include <tbb/tbb.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#define num_thr 4

double rotate(const Point& p, const Point& q, const Point& i) {
  return (q.x - p.x) * (i.y - q.y) - (q.y - p.y) * (i.x - q.x);
}

std::vector<Point> buildHull_seq(std::vector<Point> m_set) {
  if (m_set.size() < 3) throw - 1;
  Point left_point = *std::min_element(
      m_set.begin(), m_set.end(), [](Point a, Point b) { return (a < b); });

  std::vector<Point> hull;
  Point end_point;

  do {
    hull.push_back(left_point);
    end_point = m_set[0];
    for (auto i : m_set) {
      if (rotate(hull.back(), end_point, i) < 0 || end_point == left_point) {
        end_point = i;
      }
    }
    left_point = end_point;
  } while (end_point != hull[0]);
  return hull;
}

/*class reduce_ {
  std::vector<Point> m_set_;
  std::vector<Point> hull_;
  Point left_point_;
  Point res;

 public:
  reduce_(std::vector<Point> m_set, std::vector<Point> hull)
      : m_set_(m_set), hull_(hull), left_point_(hull.back()), res(m_set[0]) {}
  reduce_(reduce_& r, tbb::split)
      : m_set_(r.m_set_),
        hull_(r.hull_),
        left_point_(r.left_point_),
        res(r.res) {}

  void operator()(const tbb::blocked_range<size_t>& r) {
    for (auto i = r.begin(); i != r.end(); i++) {
      if (rotate(hull_.back(), res, m_set_[i]) < 0 || res == left_point_) {
        res = m_set_[i];
      }
    }
  }

  void join(const reduce_& r) {
    if (rotate(hull_.back(), res, r.res) < 0 || res == left_point_) {
      res = r.res;
    }
  }

  Point getPoint() { return res; }
};*/

/*std::vector<Point> buildHull_tbb(std::vector<Point> m_set) {
  int size = m_set.size();
  if (size < 3) throw - 1;
  Point left_point;

#pragma omp parallel shared(left_point)
  left_point =
      *std::min_element(m_set.begin(), m_set.end(), [](Point a, Point b) {
        return (a.y < b.y) || (a.y == b.y && a.x < b.x);
      });

  std::vector<Point> hull;
  Point end_point;

  do {
    hull.push_back(left_point);
    end_point = m_set[0];
    reduce_ r(m_set, hull);
    tbb::task_scheduler_init init(num_thr);
    int grainsize = size / num_thr + size % num_thr;
    tbb::parallel_reduce(tbb::blocked_range<size_t>(0, size, grainsize), r);
    init.terminate();
    end_point = r.getPoint();
    left_point = end_point;
  } while (end_point != hull[0]);
  return hull;
}
*/

std::vector<std::vector<Point>> splitVector(const std::vector<Point>& vec,
                                            size_t n) {
  std::vector<std::vector<Point>> outVec;
  size_t length = vec.size() / n;
  size_t remain = vec.size() % n;
  size_t begin = 0;
  size_t end = 0;

  for (size_t i = 0; i < fmin(n, vec.size()); ++i) {
    end += (remain > 0) ? (length + !!(remain--)) : length;
    outVec.push_back(
        std::vector<Point>(vec.begin() + begin, vec.begin() + end));
    begin = end;
  }
  return outVec;
}

std::vector<Point> mergeVector(std::vector<std::vector<Point>> vec) {
  std::vector<Point> res;
  std::cout << "lol";
  for (auto i : vec) {
    for (auto j : i) {
      res.push_back(j);
    }
  }

  res = buildHull_seq(res);
  return res;
}

class pTbb {
  const std::vector<std::vector<Point>>& vec;
  std::vector<std::vector<Point>>* res;

 public:
  pTbb(const std::vector<std::vector<Point>>& vec_,
       std::vector<std::vector<Point>>* res_)
      : vec(vec_), res(res_) {}
  void operator()(const tbb::blocked_range<size_t>& r) const {
    for (auto i = r.begin(); i != r.end(); i++) {
      res->push_back(buildHull_seq(vec[i]));
    }  //здесь все ок
  }
};

std::vector<Point> buildHull_tbb(std::vector<Point> m_set) {
  std::vector<std::vector<Point>> vec = splitVector(m_set, num_thr);
  std::vector<Point> hull;
  /*tbb::parallel_for(tbb::blocked_range<size_t>(0, vec.size(), 1),
                    [&vec, &res](const tbb::blocked_range<size_t>& r) {
                      int begin = r.begin(), end = r.end();
                      for (int i = begin; i != end; ++i)
                        res.push_back(buildHull_seq(vec[i]));
                    });*/

  std::vector<std::vector<Point>> res;
  tbb::parallel_for(tbb::blocked_range<size_t>(0, vec.size()), pTbb(vec, &res));
  for (auto i : res) {  //этот вывод не работает уже
    for (auto j : i) {
      std::cout << j.x << " " << j.y << " ";
    }
    std::cout << std::endl;
  }
  hull = mergeVector(res);
  return hull;
}

std::vector<Point> randomSet(int point_count) {
  if (point_count < 3) throw - 1;
  std::srand(std::time(nullptr));
  std::vector<Point> result(point_count);

  std::generate(result.begin(), result.end(), []() {
    return Point{static_cast<double>(std::rand() % 100 - 50),
                 static_cast<double>(std::rand() % 100 - 50)};
  });
  return result;
}
