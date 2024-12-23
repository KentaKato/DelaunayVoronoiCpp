#include <opencv2/opencv.hpp>

#include "DelaunayTriangulation/voronoi_diagram.hpp"
#include "DelaunayTriangulation/geometry_primitives.hpp"


namespace delaunay_triangulation
{

std::unordered_map<Site, Cell> VoronoiDiagram::create(const std::vector<Triangle> & delaunay_triangles)
{
    std::unordered_map <Site, Cell> voronoi_cells;

    // Store circumcenters around each vertex
    for (const auto &t : delaunay_triangles)
    {
        voronoi_cells[t.v1].push_back(t.circum_circle.center);
        voronoi_cells[t.v2].push_back(t.circum_circle.center);
        voronoi_cells[t.v3].push_back(t.circum_circle.center);
    }

    // Sort circumcenters in counterclockwise order
    for (auto& [site, circumcenters] : voronoi_cells) {

        std::sort(circumcenters.begin(), circumcenters.end(),
            [&site](const Vertex& a, const Vertex& b) {
                double angle_a = atan2(a.y - site.y, a.x - site.x);
                double angle_b = atan2(b.y - site.y, b.x - site.x);
                return angle_a < angle_b;
            }
        );
    }

    return voronoi_cells;
}

void VoronoiDiagram::computeVoronoiCentroids(
    const DelaunayTriangulation &delaunay,
    const std::unordered_map<Point, double> &weight_map,
    std::unordered_map<Site, Centroid> &voronoi_centroids)
{
    voronoi_centroids.clear();

    std::vector<Point> points;
    points.reserve(weight_map.size());
    for (const auto & [point, _] : weight_map)
    {
        points.push_back(point);
    }

    std::unordered_map<Point, Site> belonging_cells;
    createBelongingCellMap(delaunay, points, belonging_cells);
    std::unordered_map<Site, double> total_weight;
    for (const auto &site : delaunay.getVertices())
    {
        total_weight[site] = 0.0;
        voronoi_centroids[site] = Centroid{0, 0};
    }

    for (const auto & [point, weight] : weight_map)
    {
        const auto &belonging_cell = belonging_cells[point];
        auto & centroid = voronoi_centroids[belonging_cell];
        centroid.x += point.x * weight;
        centroid.y += point.y * weight;
        total_weight[belonging_cell] += weight;
    }
    for (auto & [site, centroid] : voronoi_centroids)
    {
        double weight = total_weight[site];
        if (weight == 0.0)
        {
            centroid = site;
            continue;
        }
        centroid.x /= weight;
        centroid.y /= weight;
    }
}

void VoronoiDiagram::createBelongingCellMap(
    const DelaunayTriangulation &delaunay,
    const std::vector<Point> &points,
    std::unordered_map<Point, Site> &belonging_cells)
{
    Site site_of_belonging_cell;
    Vertex seed_vertex = delaunay.getVertices().front();
    for (const auto &p : points)
    {
        const auto nearest_v = delaunay.findNearestVertex(p, seed_vertex);
        belonging_cells[p] = nearest_v;
        seed_vertex = nearest_v;
    }
}

void VoronoiDiagram::findBelongingCell(
    const std::vector<Site> &sites,
    const Point &p, Site &site_of_belonging_cell)
{
    auto distance2 = [](const Vertex &a, const Vertex &b) -> double {
        return pow(a.x - b.x, 2) + pow(a.y - b.y, 2);
    };
    double min_distance = std::numeric_limits<double>::max();
    for (const auto &site : sites)
    {
        double d = distance2(p, site);
        if (d < min_distance)
        {
            min_distance = d;
            site_of_belonging_cell = site;
        }
    }
}


void VoronoiDiagram::draw(
    cv::Mat &img,
    const std::unordered_map<Vertex, std::vector<Vertex>> &voronoi_cells,
    const cv::Scalar &color)
{
    for (const auto& [vertex, circumcenters] : voronoi_cells) {
        std::vector<cv::Point> points;
        points.reserve(circumcenters.size());
        for (const auto& circumcenter : circumcenters) {
            points.push_back(cv::Point(circumcenter.x, circumcenter.y));
            cv::circle(img, cv::Point(circumcenter.x, circumcenter.y), 4, color, -1);
        }
        cv::polylines(img, points, true, color, 1, cv::LINE_AA);
    }
}



} // namespace delaunay_Triangulation