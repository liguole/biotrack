#ifndef TRACK_H
#define TRACK_H


#include <QtCore>
#include <QtGui>
#include <utility>


/** OPENCV INCLUDES**/
#if defined(WIN32) && !defined(linux)
#include "opencv.hpp" //FOR WINDOWS
#elif defined(linux) && !defined(WIN32)
#include <cv.h> //for linux
#include <highgui.h> //for linux

#else
//Error! Got to be linux or windows not both or neither (or i guess mac!)
#include <cv.h>
#include <highgui.h>
#endif
/** END OPENCV INCLUDES**/


#include <vector>
#include <stdio.h>

#include <iostream>
#include "pcl/kdtree/impl/kdtree_flann.hpp" //this include has to go above others to work!
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>
#include<pcl/search/pcl_search.h>


#include <pcl/filters/radius_outlier_removal.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/extract_indices.h>

#include <fstream>
#include <pcl/common/angles.h>
#include <pcl/console/parse.h>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/point_representation.h>

#include <pcl/common/point_operators.h>
#include <pcl/io/pcd_io.h>
#include <pcl/keypoints/uniform_sampling.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/fpfh.h>
#include <pcl/registration/correspondence_estimation.h>
#include <pcl/registration/correspondence_estimation_normal_shooting.h>
//#include <pcl/registration/correspondence_estimation_backprojection.h>
#include <pcl/registration/correspondence_rejection_distance.h>
#include <pcl/registration/correspondence_rejection_median_distance.h>
#include <pcl/registration/correspondence_rejection_surface_normal.h>
#include <pcl/registration/transformation_estimation_point_to_plane_lls.h>
#include <pcl/registration/transformation_estimation.h>
//#include <pcl/registration/impl/default_convergence_criteria.h>

#include <pcl/visualization/pcl_visualizer.h>

#include <pcl/console/parse.h>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/point_representation.h>

#include <pcl/io/pcd_io.h>
#include <pcl/ros/conversions.h>
#include <pcl/keypoints/uniform_sampling.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/fpfh.h>
#include <pcl/registration/correspondence_estimation.h>
#include <pcl/registration/correspondence_rejection_distance.h>
#include <pcl/registration/transformation_estimation_svd.h>



//To visualize Clouds
#include <pcl/visualization/cloud_viewer.h>

#include <QDebug>
//#include "ui_Manytrack.h"

#include "icp_color.h"
#include "trans_2D/transformation_estimation_2D.h"



using namespace cv;
using namespace std;
using namespace pcl;
using namespace Eigen;

typedef struct Model
{
    Mat img;
    Point centroid;
    pcl::PointCloud<pcl::PointXYZRGB> cloud;
    QString name;
    QString filepath;
    int width;
    int height;
    int maxDimension;
    float rotated;

}
Model;

/**
 * This class is responsible for managing the state of an ant
 * track.  The state of a track is the frame index of birth
 * and death, centroid at birthtime.
 * Rotation, timestamp, translation, and ID information are kept for each track
 */
class Track
{

public:
    Track(int index, pcl::PointXYZRGB initTranslation, int identification, double matchDThresh,int sepThresh, int resFracMultiplier);
    ~Track();



    double getX(int idx=-1);
    double getY(int idx=-1);
    pair <Point,double> getXYT(int idx=-1);

    double getScale(int idx=-1);
    double getRotationAngle(int idx=-1);
    void getTemplatePoints(pcl::PointCloud<pcl::PointXYZRGB>& modelPts, int idx=-1);
    bool isBirthFrame;
    pcl::PointCloud<pcl::PointXYZRGB> updatePosition(pcl::PointCloud<pcl::PointXYZRGB> dataPTS_cloud,    vector<Model> modelPTS_clouds,
                                          int areaThreshold,  int separateThreshold, int matchDThresh,
                                          int ICP_ITERATIONS, double ICP_TRANSEPSILON, double ICP_EUCLIDEANDIST, double ICP_MAXFIT);

    int identify (PointCloud<PointXYZRGB> dataPTS_cloud, vector<Model> modelgroup);

    int icp_maxIter;
    double icp_transformationEpsilon;
    double icp_euclideanDistance;
    double  icp_maxfitness;
    int getBirthFrameIndex() { return birthFrameIndex; }
    int getDeathFrameIndex() { return deathFrameIndex; }
    int getFrameIndex() { return frameIndex; }
    int getLength() { return absoluteTransforms.size(); }
    int getFrameIndexOfLastUpdate() { return birthFrameIndex + absoluteTransforms.size()-1; }
    void end() { deathFrameIndex = birthFrameIndex + absoluteTransforms.size()-1; }
    bool isActive() { return deathFrameIndex > 0; }
    int getID() { return IDnum; }
    bool wasBirthed() { return isBirthable; }
    int getNumberOfContinuousZombieFrames() { return numberOfContinuousZombieFrames; }
    int birthFrameIndex;
    int frameIndex;
    int deathFrameIndex;
    int IDnum;
    QString modelType;
    int modelIndex;
    double matchDistanceThreshold;
    double nukeDistanceThreshold; // used when a neighborhood of pts close to a model point is nuked

    double recentFitness;

//    Correspondences stuff
    void estimateKeypoints (const PointCloud<PointXYZRGB>::Ptr &src,
                       const PointCloud<PointXYZRGB>::Ptr &tgt,
                       PointCloud<PointXYZRGB> &keypoints_src,
                       PointCloud<PointXYZRGB> &keypoints_tgt);
    void estimateNormals (const PointCloud<PointXYZRGB>::Ptr &src,
                     const PointCloud<PointXYZRGB>::Ptr &tgt,
                     PointCloud<Normal> &normals_src,
                     PointCloud<Normal> &normals_tgt);
    void estimateFPFH (const PointCloud<PointXYZRGB>::Ptr &src,
                       const PointCloud<PointXYZRGB>::Ptr &tgt,
                       const PointCloud<Normal>::Ptr &normals_src,
                       const PointCloud<Normal>::Ptr &normals_tgt,
                       const PointCloud<PointXYZRGB>::Ptr &keypoints_src,
                       const PointCloud<PointXYZRGB>::Ptr &keypoints_tgt,
                       PointCloud<FPFHSignature33> &fpfhs_src,
                       PointCloud<FPFHSignature33> &fpfhs_tgt);
  void  findCorrespondences (const PointCloud<FPFHSignature33>::Ptr &fpfhs_src,
                         const PointCloud<FPFHSignature33>::Ptr &fpfhs_tgt,
                         Correspondences &all_correspondences);

  void  rejectBadCorrespondences (const CorrespondencesPtr &all_correspondences,
                              const PointCloud<PointXYZRGB>::Ptr &keypoints_src,
                              const PointCloud<PointXYZRGB>::Ptr &keypoints_tgt,
                              Correspondences &remaining_correspondences);

  void  computeTransformation (const PointCloud<PointXYZRGB>::Ptr &src,
                           const PointCloud<PointXYZRGB>::Ptr &tgt,
                           Eigen::Matrix4f &transform);

  void
  icp (const PointCloud<Point>::Ptr &src,
       const PointCloud<Point>::Ptr &tgt,
       Eigen::Matrix4d &transform);
  Eigen::Matrix4f calcTransformPCLRGB(pcl::PointCloud<pcl::PointXYZRGB> data_cloud,pcl::PointCloud<pcl::PointXYZRGB> model_cloud,double *fitness);

  float modelRotated;

private:
    //Ui::ManytrackClass uitrack;

    pcl::PointXYZRGB initialTranslation;
    std::vector<Eigen::Matrix4f> transforms; // vector of incremental transformation matrices State = T_n * (T_n-1 * ... T_2 * T_1) * I
    std::vector<int> zombieIndicesIntoAbsTransforms; //notes which abs transforms are zombies (ie transitioning to death)
    int numberOfContinuousZombieFrames;
    bool isBirthable;
    // Absolute transformation matrices are useful for random frame access
    std::vector<Eigen::Matrix4f> absoluteTransforms; // vector of absolute transformation matrices State = T_x * I where x is any state //TODO update this

    //ICP alignment
    Eigen::Matrix4f update2DTransformPCL(pcl::PointCloud<pcl::PointXY> data_cloud,pcl::PointCloud<pcl::PointXY> model_cloud);
//        Eigen::Matrix4f updateTransformPCL(pcl::PointCloud<PointT> data_cloud,pcl::PointCloud<PointT> model_cloud);

    double matchScore;
    bool didConverge;


    void transformPoints(std::vector<Point>& modelPts, Mat transform);
    void transformCloud(pcl::PointCloud<pcl::PointXYZRGB> modelPTS_cloud, Eigen::Matrix4f transform);
    pcl::PointCloud<pcl::PointXYZRGB> removeClosestDataCloudPoints(pcl::PointCloud<pcl::PointXYZRGB> point_cloud_for_reduction,pcl::PointCloud<pcl::PointXYZRGB> removal_Cloud, int distanceThreshold);
    void removeClosestDataPoints(std::vector<Point> &reducedPts, Point queryPoint, int distanceThreshold);
    int  resolutionFracMultiplier;

    pcl::PointCloud<pcl::PointXYZRGB> tformedModel_cloud180;






protected:

};

class Remove_Parallel : public cv::ParallelLoopBody
{
private:
    bool *marked;//= new bool[point_cloud_for_reduction.size()];
//    memset(marked,false,sizeof(bool)*point_cloud_for_reduction.size());

    pcl::KdTreeFLANN<pcl::PointXYZRGB> * kdtree;
    pcl::PointCloud<pcl::PointXYZRGB> removal_Cloud,point_cloud_for_reduction;

//     std::vector<int> pointIdxRadiusSearch;
//          std::vector<float> pointRadiusSquaredDistance;
          double point_radius;

public:


    Remove_Parallel ( pcl::KdTreeFLANN<pcl::PointXYZRGB> * thekdtree, pcl::PointCloud<pcl::PointXYZRGB>  theremoval_Cloud,
                      pcl::PointCloud<pcl::PointXYZRGB> thepoint_cloud_for_reduction,
//                      std::vector<int> thepointIdxRadiusSearch,   std::vector<float> thepointRadiusSquaredDistance,
                      double thepoint_radius, bool** themarked )
        : kdtree(thekdtree), removal_Cloud(theremoval_Cloud), point_cloud_for_reduction(thepoint_cloud_for_reduction),
//          pointIdxRadiusSearch(thepointIdxRadiusSearch), pointRadiusSquaredDistance(thepointRadiusSquaredDistance),
          point_radius(thepoint_radius)

    {
        *themarked= new bool[point_cloud_for_reduction.size()];
        memset(*themarked,false,sizeof(bool)*point_cloud_for_reduction.size());
        marked = *themarked;

    }





void operator ()(const cv::Range& range) const
{

//0266201780

    std::vector<int> pointIdxRadiusSearch_loop;
         std::vector<float> pointRadiusSquaredDistance_loop;
         double point_radius_loop;

//         pointIdxRadiusSearch_loop = pointIdxRadiusSearch;
//         pointRadiusSquaredDistance_loop = pointRadiusSquaredDistance;
         point_radius_loop = point_radius;
         pcl::KdTreeFLANN<pcl::PointXYZRGB> * kdtree_loop;
         kdtree_loop=kdtree;




                for (size_t c = range.start; c!=range.end;++c){ // This goes over the size of the cloud
                    if(point_cloud_for_reduction.size()<1){
                        break;
                    }


                    pcl::PointXYZRGB searchPoint;

                    searchPoint.x = removal_Cloud.points[c].x;
                    searchPoint.y = removal_Cloud.points[c].y;
                   //Need to take z as zero when using a flattened data point cloud
                   searchPoint.z = 0;

                    // qDebug() <<"Datapts before incremental remove"<< point_cloud_for_reduction.size();
                   int kdtest=-1;
                  kdtest= kdtree_loop->radiusSearch ( searchPoint, point_radius_loop, pointIdxRadiusSearch_loop, pointRadiusSquaredDistance_loop);

                  int doanotherthingtest;

                    if ( kdtest>0)  {
                        for (size_t i = 0; i < pointIdxRadiusSearch_loop.size (); ++i){
                            if(point_cloud_for_reduction.size()>0) ///NOTE CHANGED FROM > 1

                            marked[pointIdxRadiusSearch_loop[i]]=true;
            //                        point_cloud_for_reduction.erase(point_cloud_for_reduction.begin()+pointIdxRadiusSearch[i]);// point_cloud_for_reduction.points[ pointIdxRadiusSearch[i] ]
                        }
                    }

                }
}

};


class Identify_Parallel : public cv::ParallelLoopBody
{
private:
pcl:: PointCloud<PointXYZRGB> dataPTS_cloud;
double fit;
int identitynum;
pair<int,double>* idscores;

vector<Model> modelgroup;

Track* atrack;

public:




Identify_Parallel(PointCloud<PointXYZRGB> data_cloud,vector<Model> allmodelgroup, Track* thetrack,pair<int,double>** idandscores, int rangesize)
        : dataPTS_cloud(data_cloud),  modelgroup( allmodelgroup), atrack(thetrack) //,idscores(idandscores)
    {
//    idscores = new pair<int,double>*[rangesize];

//    pairArr = new std::pair<double,double> [3];

    *idandscores = new pair<int,double> [rangesize];
    idscores = *idandscores;

//idscore
    }


    void operator ()(const cv::Range& range) const
    {

//        vector<pair<int,double> > *id_scores = idscores;
//        id_scores->reserve(range.size()-1);

//        pair <int,double> * id_score_arr[range.size()]; //Is this how to initialize an array of pairs?
//        pair <int,double> *id_score_arr[];
pair <int,double> * id_score_arr = idscores;
//        pair<int,double> id_score_arr = new pair<int,double>[range.size()]; //UPDATE, THIS GIVES ERROS (pointer stuff) or is this more proper?





        ///   Check the fit of different models in parallel
                for (size_t modelgroupnum = range.start; modelgroupnum!= range.end; ++modelgroupnum){ //Not sure if we need -1?

            PointCloud<PointXYZRGB> modelTOIdentify = modelgroup[modelgroupnum].cloud;

            double recentfitness;

            atrack->calcTransformPCLRGB(dataPTS_cloud, modelTOIdentify, &recentfitness);


            pair<int,double> id_score;

            id_score.first=modelgroupnum;
            id_score.second=recentfitness;

            id_score_arr[modelgroupnum].first = modelgroupnum;
            id_score_arr[modelgroupnum].second = recentfitness;
//            qDebug()<<"modegroupnum "<<modelgroupnum<<"    idscore_arr first "<<id_score_arr[modelgroupnum].first<<"  second "<<id_score_arr[modelgroupnum].second;


//                    = id_score;
//            id_scores->push_back(id_score); // FIX TODO Heads up, this can be a problem point at random times

        }

    }




};

#endif // TRACK_H
