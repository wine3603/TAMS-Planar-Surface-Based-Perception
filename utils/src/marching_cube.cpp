#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/surface/marching_cubes.h>
#include <pcl/surface/marching_cubes_greedy.h>
#include <pcl/surface/marching_cubes_greedy_dot.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/feature.h>
#include <pcl/io/vtk_io.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/surface/mls.h>
int
main (int argc, char **argv)
{
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
  if (argc != 2)
  {
    std::cout << "usage: marching_cube <pcdfile>.\n";
    return -1;
  }
  std::string pcd_file(argv[1]);
  if (pcl::io::loadPCDFile<pcl::PointXYZ> (argv[1], *cloud) == -1) //* load the file
  {
    PCL_ERROR ("Couldn't read file %s!\n", pcd_file.c_str());
    return (-1);
  }
  PCL_INFO ("Loaded %d points with width: %d and height: %d from %s.\n",
            cloud->points.size(), cloud->width, cloud->height, pcd_file.c_str ());

  // Create a KD-Tree
  pcl::search::KdTree<pcl::PointXYZ>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZ>);
  // Output has the same type as the input one, it will be only smoothed
  pcl::PointCloud<pcl::PointXYZ> mls_points;
  // Init object (second point type is for the normals, even if unused)
  pcl::MovingLeastSquares<pcl::PointXYZ, pcl::Normal> mls;
  // Optionally, a pointer to a cloud can be provided, to be set by MLS
  pcl::PointCloud<pcl::Normal>::Ptr mls_normals (new pcl::PointCloud<pcl::Normal> ());
  mls.setOutputNormals (mls_normals);
  // Set parameters
  mls.setInputCloud (cloud);
  mls.setPolynomialFit (true);
  mls.setSearchMethod (tree);
  mls.setSearchRadius (0.1);
  // Reconstruct
  mls.reconstruct (mls_points);
  // Concatenate fields for saving
  pcl::PointCloud<pcl::PointNormal>::Ptr mls_cloud (new pcl::PointCloud<pcl::PointNormal>);
  pcl::concatenateFields (mls_points, *mls_normals, *mls_cloud);
  PCL_INFO ("[MovingLeastSquares] %d points remained after smoothed using MovingLeastSquares.\n", mls_points.size());
//  // Normal estimation*
//  pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> n;
//  pcl::PointCloud<pcl::Normal>::Ptr normals (new pcl::PointCloud<pcl::Normal>);
//  pcl::search::KdTree<pcl::PointXYZ>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZ>);
//  tree->setInputCloud (cloud);
//  n.setInputCloud (cloud);
//  n.setSearchMethod (tree);
//  n.setKSearch (20);
//  n.compute (*normals);
//  // Concatenate the XYZ and normal fields*
//  pcl::PointCloud<pcl::PointNormal>::Ptr cloud_with_normals(new pcl::PointCloud<pcl::PointNormal>);
//  pcl::concatenateFields(*cloud, *normals,*cloud_with_normals);
   pcl::PolygonMesh mesh;
   pcl::search::KdTree<pcl::PointNormal>::Ptr tree2 (new pcl::search::KdTree<pcl::PointNormal>);
   tree2->setInputCloud (mls_cloud);
   // Initialize objects
   pcl::MarchingCubesGreedyDot<pcl::PointNormal> mc;
   // Set parameters
   doubleleafSize = 0.05;
   doubleisoLevel = 0.5;
   mc.setLeafSize(leafSize);
   mc.setIsoLevel(isoLevel);   //ISO: must be between 0 and 1.0
   mc.setSearchMethod(tree2);
   mc.setInputCloud(mls_cloud);
   // Reconstruct
   mc.reconstruct (mesh);
   //Saving to disk in VTK format:
   pcl::io::saveVTKFile ("mesh.vtk", mesh);
   //visualizing the mesh
   int viewports[2] = {1,2};
   pcl::visualization::PCLVisualizer viewer ("mesh generated by marching cubes");
   viewer.setBackgroundColor (0.0, 0.0, 0.0);
   viewer.createViewPort(0, 0, 0.5, 1.0, viewports[0]);
   viewer.createViewPort(0.5, 0, 1.0, 1.0, viewports[1]);
   viewer.addPointCloud(cloud, "cloud", viewports[0]);
   viewer.addPolygonMesh (mesh, "mesh", viewports[1]);
   viewer.addCoordinateSystem (1.0);
   viewer.setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "cloud");
   viewer.spin ();
   return 0;
}
