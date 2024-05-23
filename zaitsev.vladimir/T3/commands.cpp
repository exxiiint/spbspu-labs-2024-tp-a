#include "commands.hpp"
#include <iterator>
#include <algorithm>
#include <functional>
#include <string>
#include <iomanip>
#include <numeric>
#include <cmath>
#include <stream_guard.hpp>

namespace plh = std::placeholders;

class Generator
{
public:
  Generator(const zaitsev::Polygon& base);
  std::pair< zaitsev::Point, zaitsev::Point > operator()();
private:
  const std::vector< zaitsev::Point >& points;
  size_t pos;
};
int skew_product(std::pair< zaitsev::Point, zaitsev::Point > vects);
double getArea(const zaitsev::Polygon& poly);
double getCondAreaSum(const std::list< zaitsev::Polygon >& polys, std::function< bool(const zaitsev::Polygon&) > cond);
bool isOddSize(const zaitsev::Polygon& poly);
bool isEvenSize(const zaitsev::Polygon& poly);
bool isEqualSize(const zaitsev::Polygon& poly, size_t target);
bool getTrueCond(const zaitsev::Polygon& poly);
bool compareSize(const zaitsev::Polygon& p1, const zaitsev::Polygon& p2);
bool compareArea(const zaitsev::Polygon& p1, const zaitsev::Polygon& p2);
std::pair< zaitsev::Point, zaitsev::Point > getFrameRect(const std::vector< zaitsev::Point >& pts);
std::pair< zaitsev::Point, zaitsev::Point > uniteFrameRects(const std::list< zaitsev::Polygon >& poly);
bool checkOutOfBounds(std::pair< zaitsev::Point, zaitsev::Point > bounds, zaitsev::Point pt);

std::ostream& zaitsev::processAreaCmd(std::istream& in, std::ostream& out, std::list< Polygon >& shapes)
{
  StreamGuard guard(out);
  out << std::fixed << std::setprecision(1);
  std::string arg;
  in >> arg;
  if (arg == "EVEN")
  {
    return out << getCondAreaSum(shapes, isEvenSize) << '\n';
  }
  if (arg == "ODD")
  {
    return out << getCondAreaSum(shapes, isOddSize) << '\n';
  }
  if (!shapes.empty() && arg == "MEAN")
  {
    return out << getCondAreaSum(shapes, getTrueCond) / shapes.size() << '\n';
  }
  size_t sz = std::stoull(arg);
  if (sz < 3)
  {
    throw std::invalid_argument("");
  }
  std::function< bool(const Polygon&) > cond = std::bind(isEqualSize, plh::_1, sz);
  return out << getCondAreaSum(shapes, cond) << '\n';
}

std::ostream& zaitsev::processMaxCmd(std::istream& in, std::ostream& out, std::list< Polygon >& shapes)
{
  std::string arg;
  in >> arg;
  if (!shapes.empty() && arg == "AREA")
  {
    StreamGuard guard(out);
    out << std::fixed << std::setprecision(1);
    return out << getArea(*(std::max_element(shapes.begin(), shapes.end(), compareArea))) << '\n';
  }
  if (!shapes.empty() && arg == "VERTEXES")
  {
    return out << std::max_element(shapes.begin(), shapes.end(), compareSize)->points.size() << '\n';
  }
  throw std::invalid_argument("");
}

std::ostream& zaitsev::processMinCmd(std::istream& in, std::ostream& out, std::list< Polygon >& shapes)
{
  std::string arg;
  in >> arg;
  if (!shapes.empty() && arg == "AREA")
  {
    StreamGuard guard(out);
    out << std::fixed << std::setprecision(1);
    return out << getArea(*(std::min_element(shapes.begin(), shapes.end(), compareArea))) << '\n';
  }
  if (!shapes.empty() && arg == "VERTEXES")
  {
    return out << std::min_element(shapes.begin(), shapes.end(), compareSize)->points.size() << '\n';
  }
  throw std::invalid_argument("");
}

std::ostream& zaitsev::processCountCmd(std::istream& in, std::ostream& out, std::list< Polygon>& shapes)
{
  std::string arg;
  in >> arg;
  if (arg == "EVEN")
  {
    return out << std::count_if(shapes.begin(), shapes.end(), isEvenSize) << '\n';
  }
  if (arg == "ODD")
  {
    return out << std::count_if(shapes.begin(), shapes.end(), isOddSize) << '\n';
  }
  size_t sz = std::stoull(arg);
  if (sz < 3)
  {
    throw std::invalid_argument("");
  }
  std::function< bool(const Polygon&) > cond = std::bind(isEqualSize, plh::_1, sz);
  return out << std::count_if(shapes.begin(), shapes.end(), cond) << '\n';
}

std::ostream& zaitsev::processLessareaCmd(std::istream& in, std::ostream& out, std::list< Polygon>& shapes)
{
  Polygon target;
  in >> target;
  if (!in)
  {
    throw std::invalid_argument("");
  }
  std::function< bool(const Polygon&) > cond = std::bind(compareArea, plh::_1, target);
  return out << std::count_if(shapes.begin(), shapes.end(), cond) << '\n';
}

std::ostream& zaitsev::processInframeCmd(std::istream& in, std::ostream& out, std::list< Polygon>& shapes)
{
  Polygon p;
  in >> p;
  if (!in)
  {
    throw std::invalid_argument("");
  }
  std::pair< Point, Point> bounds = uniteFrameRects(shapes);
  std::function< bool(Point) > cond = std::bind(checkOutOfBounds, bounds, plh::_1);
  return out << (std::find_if(p.points.begin(), p.points.end(), cond) == p.points.end() ? "<TRUE>\n" : "<FALSE>\n");
}

int skew_product(std::pair< zaitsev::Point, zaitsev::Point > vects)
{
  return vects.first.x * vects.second.y - vects.first.y * vects.second.x;
}


Generator::Generator(const zaitsev::Polygon& base):
  points(base.points),
  pos(0)
{}

std::pair< zaitsev::Point, zaitsev::Point > Generator::operator()()
{
  pos = (pos + 1) % points.size();
  return { points[pos], points[(pos + 1) % points.size()] };
}

double getArea(const zaitsev::Polygon& poly)
{
  using namespace std::placeholders;
  std::list< std::pair< zaitsev::Point, zaitsev::Point > > edges;
  Generator gen(poly);
  std::generate_n(std::back_inserter(edges), poly.points.size(), gen);
  std::list< int > areas;
  std::transform(edges.begin(), edges.end(), std::back_inserter(areas), skew_product);
  return std::fabs(std::accumulate(areas.begin(), areas.end(), 0) / 2.0);
}

double getCondAreaSum(const std::list< zaitsev::Polygon >& polys, std::function< bool(const zaitsev::Polygon&) > cond)
{
  std::list< zaitsev::Polygon >satisfy_cond;
  std::copy_if(polys.begin(), polys.end(), std::back_inserter(satisfy_cond), cond);
  std::list< double > areas;
  std::transform(satisfy_cond.begin(), satisfy_cond.end(), std::back_inserter(areas), getArea);
  return std::accumulate(areas.begin(), areas.end(), 0.0);
}

bool isOddSize(const zaitsev::Polygon& poly)
{
  return poly.points.size() % 2 == 1;
}
bool isEvenSize(const zaitsev::Polygon& poly)
{
  return poly.points.size() % 2 == 0;
}
bool isEqualSize(const zaitsev::Polygon& poly, size_t target)
{
  return poly.points.size() == target;
}
bool getTrueCond(const zaitsev::Polygon& poly)
{
  return std::addressof(poly) == std::addressof(poly);
}
bool compareSize(const zaitsev::Polygon& p1, const zaitsev::Polygon& p2)
{
  return p1.points.size() < p2.points.size();
}
bool compareArea(const zaitsev::Polygon& p1, const zaitsev::Polygon& p2)
{
  return  getArea(p1) < getArea(p2);
}
bool checkOutOfBounds(std::pair< zaitsev::Point, zaitsev::Point > bounds, zaitsev::Point pt)
{
  return (pt.x < bounds.first.x || pt.x > bounds.second.x || pt.y < bounds.first.y || pt.y > bounds.second.y);
}

std::pair< zaitsev::Point, zaitsev::Point > getFrameRect(const std::vector< zaitsev::Point >& pts)
{
  using namespace zaitsev;
  auto cmp_x = [](Point lhs, Point rhs)
    {
      return lhs.x < rhs.x;
    };
  auto cmp_y = [](Point lhs, Point rhs)
    {
      return lhs.y < rhs.y;
    };
  auto bounds_x = std::minmax_element(pts.begin(), pts.end(), cmp_x);
  auto bounds_y = std::minmax_element(pts.begin(), pts.end(), cmp_y);
  return { { bounds_x.first->x, bounds_y.first->y }, { bounds_x.second->x, bounds_y.second->y } };
}

std::pair< zaitsev::Point, zaitsev::Point > uniteFrameRects(const std::list< zaitsev::Polygon >& poly)
{
  using namespace zaitsev;
  using rect_bounds = std::pair< Point, Point >;
  auto get_min = [](rect_bounds bounds)
    {
      return bounds.first;
    };
  auto get_max = [](rect_bounds bounds)
    {
      return bounds.second;
    };
  auto wrapper = [](const Polygon& poly)
    {
      return getFrameRect(poly.points);
    };
  std::list< rect_bounds > bounds;
  std::vector< Point> bound_points;
  std::transform(poly.begin(), poly.end(), std::back_inserter(bounds), wrapper);
  std::transform(bounds.begin(), bounds.end(), std::back_inserter(bound_points), get_min);
  std::transform(bounds.begin(), bounds.end(), std::back_inserter(bound_points), get_max);
  return getFrameRect(bound_points);
}

