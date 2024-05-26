#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <vector>
#include "polygon.hpp"

namespace sivkov
{
  struct BoundingBox
  {
    int minX;
    int maxX;
    int minY;
    int maxY;
  };

  struct CountAreaTriangle
  {
    Point first;
    Point second;

    double operator()(const Point& third);
  };

  struct MinArea
  {
    bool operator()(const Polygon& p1, const Polygon& p2) const;
  };

  struct MinVertices
  {
    bool operator()(const Polygon& p1, const Polygon& p2) const;
  };

  struct MaxArea
  {
    bool operator()(const Polygon& p1, const Polygon& p2) const;
  };

  struct MaxVertices
  {
    bool operator()(const Polygon& p1, const Polygon& p2) const;
  };

  BoundingBox calculateBoundingBox(const std::vector< Polygon >& polygons);
  bool isPolygonInsideBoundingBox(const Polygon& polygon, const BoundingBox& bbox);
  double countAreaShape(const Polygon& shape);
  bool compare(const Polygon& shape, size_t num);
  bool isEvenVertices(const Polygon& polygon);
  bool isOddVertices(const Polygon& polygon);
  bool hasVertexCount(const Polygon& polygon, size_t count);
  bool arePointsPermutations(const Polygon& p1, const Polygon& p2);
  size_t getVertexes(const Polygon& shape);
  double getOddEvenMean(std::vector< Polygon >& polygon, std::string arg);
  void minMaxAreas(std::ostream& out, const std::vector< Polygon >& polygon, const std::string& current);
  void minMaxVertexes(std::ostream& out, const std::vector< Polygon >& polygon, const std::string& current);
}
#endif
