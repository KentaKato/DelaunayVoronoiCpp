#ifndef DELAUNAY_TRIANGULATION__VORONOI_DIAGRAM_HPP
#define DELAUNAY_TRIANGULATION__VORONOI_DIAGRAM_HPP

#include <opencv2/opencv.hpp>
#include <map>


// Project
#include "DelaunayTriangulation/geometry_primitives.hpp"

namespace delaunay_triangulation
{

using Cell = std::vector<Vertex>;
using Site = Vertex;
using Centroid = Vertex;
using Point = Vertex;

class VoronoiDiagram
{
public:
    static std::map<Site, Cell> create(const std::vector<Triangle> & delaunay_triangles);
    static void computeVoronoiCentroids(
        const std::vector<Site> &sites,
        const std::map<Point, double> &weight_map,
        std::map<Site, Centroid> &voronoi_centroids);
    static void createBelongingCellMap(
        const std::vector<Site> &sites,
        const std::vector<Point> &points,
        std::map<Point, Site> &belonging_cells);
    static void findBelongingCell(
        const std::vector<Site> &sites,
        const Point &p, Site &site_of_belonging_cell);
    static void draw(
        cv::Mat &img,
        const std::map<Vertex, std::vector<Vertex>> &voronoi_cells,
        const cv::Scalar &color = cv::Scalar(30, 105, 210));
};

} // namespace delaunay_Triangulation

#endif