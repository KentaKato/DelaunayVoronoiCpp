#include <opencv2/opencv.hpp>

#include "DelaunayTriangulation/delaunay_triangulation.hpp"


namespace delaunay_triangulation
{

DelaunayTriangulation::DelaunayTriangulation()
{
}

void DelaunayTriangulation::addVertex(double x, double y)
{
    vertices_.emplace_back(x, y);
}

void DelaunayTriangulation::addVertex(const Vertex &v)
{
    vertices_.push_back(v);
    super_triangle_vertices_.clear();
}

void DelaunayTriangulation::removeLastVertex()
{
    if (!vertices_.empty())
    {
        vertices_.pop_back();
    }
}

bool DelaunayTriangulation::hasVertex(const Vertex &v) const
{
    return std::find(vertices_.begin(), vertices_.end(), v) != vertices_.end();
}

void DelaunayTriangulation::reserveVerticesVector(const size_t size)
{
    vertices_.reserve(size);
}

std::vector<Triangle> DelaunayTriangulation::getTriangles() const
{
    std::vector<Triangle> triangles = triangles_;
    triangles.erase(
        std::remove_if(triangles.begin(), triangles.end(),
            [this](const Triangle& triangle) {
                return this->isSuperTriangle(triangle);
            }),
        triangles.end()
    );
    return triangles;
}

std::vector<Triangle> DelaunayTriangulation::getSuperTriangles() const
{
    std::vector<Triangle> triangles = triangles_;
    triangles.erase(
        std::remove_if(triangles.begin(), triangles.end(),
            [this](const Triangle& triangle) {
                return !this->isSuperTriangle(triangle);
            }),
        triangles.end()
    );
    return triangles;
}

std::vector<Triangle> DelaunayTriangulation::getAllTriangles() const
{
    return triangles_;
}

void DelaunayTriangulation::createDelaunayTriangles()
{
    reset();

    if (vertices_.size() < 3)
    {
        return;
    }

    this->setupSuperTriangle();

    for (const auto &v : vertices_)
    {
        auto find_triangle_containing_vertex = [this](const Vertex &v) -> std::vector<Triangle>
        {
            std::vector<Triangle> containing_triangles;
            constexpr size_t SUFFICIENT_CAPACITY = 10;
            containing_triangles.reserve(SUFFICIENT_CAPACITY);
            for (const auto &t : triangles_)
            {
                if (t.isInCircumCircle(v))
                {
                    containing_triangles.push_back(t);
                }
            }
            return containing_triangles;
        };

        const auto containing_triangles = find_triangle_containing_vertex(v);
        const auto unshared_edges = this->parseUnsharedEdges(containing_triangles);
        this->erase(containing_triangles);
        triangles_.reserve(vertices_.size() + 3 /* super triangle vertices */);
        for (const auto &e : unshared_edges)
        {
            triangles_.emplace_back(e.v1, e.v2, v);
        }
    }
}

void DelaunayTriangulation::clear()
{
    vertices_.clear();
    triangles_.clear();
    super_triangle_vertices_.clear();
}

void DelaunayTriangulation::reset()
{
    triangles_.clear();
    super_triangle_vertices_.clear();
}

void DelaunayTriangulation::erase(const Triangle &t)
{
    std::erase(triangles_, t);
}

void DelaunayTriangulation::erase(const std::vector<Triangle> &triangles)
{
    for (const auto &t : triangles)
    {
        this->erase(t);
        std::erase(triangles_, t);
    }
}

void DelaunayTriangulation::setupSuperTriangle()
{
    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::min();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::min();
    for (const auto &v : vertices_)
    {
        if (v.x < min_x) min_x = v.x;
        if (v.x > max_x) max_x = v.x;
        if (v.y < min_y) min_y = v.y;
        if (v.y > max_y) max_y = v.y;
    }

    // Rectangle that contains all the points
    const double center_x = (min_x + max_x) / 2.0;
    const double height = max_y - min_y;
    const double width = max_x - min_x;

    const Vertex v1{center_x, min_y + 2.0 * height};
    const Vertex v2{center_x - 1.5 * width, min_y - 0.5 * height};
    const Vertex v3{center_x + 1.5 * width, min_y - 0.5 * height};
    super_triangle_vertices_.resize(3);
    super_triangle_vertices_[0] = v1;
    super_triangle_vertices_[1] = v2;
    super_triangle_vertices_[2] = v3;
    triangles_.emplace_back(v1, v2, v3);
}

bool DelaunayTriangulation::isSuperTriangle(const Triangle &t) const
{
    for (const auto &v : super_triangle_vertices_)
    {
        if (t.has(v))
        {
            return true;
        }
    }
    return false;
}

std::vector<Edge> DelaunayTriangulation::parseUnsharedEdges(const std::vector<Triangle> &triangles) const
{
    // TODO: The current computational complexity is O(n),
    //       so it should be changed to O(n) by counting the number of occurrences of edges.
    std::vector<Edge> unshared_edges;
    unshared_edges.reserve(3 * triangles.size());
    for (size_t i = 0, n = triangles.size(); i < n; ++i)
    {
        const auto & t_i = triangles[i];
        std::vector<Edge> edges{
            Edge{t_i.v1, t_i.v2},
            Edge{t_i.v2, t_i.v3},
            Edge{t_i.v3, t_i.v1}};
        for (const auto & e : edges)
        {
            bool shared = false;
            for (size_t j = 0; j < n; ++j)
            {
                if (i == j)
                {
                    continue;
                }
                const auto & t_j = triangles[j];
                if (t_j.has(e))
                {
                    shared = true;
                    break;
                }
            }
            if (!shared)
            {
                unshared_edges.push_back(e);
            }
        }
    }
    return unshared_edges;
}

} // namespace delaunay_Triangulation