/** @Converting functions between structures of openCV and KVLD suitable structures
 ** @author Zhe Liu
 **/

/*
Copyright (C) 2007-12 Zhe Liu and Pierre Moulon.
All rights reserved.

This file is part of the KVLD library and is made available under
the terms of the BSD license (see the COPYING file).
*/
#include "./kvld/spline.h"
#include "convert.h"

//=========================image convertions===================================// 
int Convert_image(const cv::Mat& In, Image<float> & imag)//convert only gray scale image of opencv
{ 
  unsigned char* pixelPtr = (unsigned char*) In.data;
  int cn=In.channels();
  imag = Image<float>(In.cols, In.rows);
  if (cn==1)//gray scale
  {
    for (int i = 0; i  < In.rows; ++i)
    {
      for (int j = 0; j  < In.cols; ++j)
      {
        imag(i,j)= float(pixelPtr[i*In.cols+ j]); 
      }
    }
  }else
  {
    for (int i = 0; i  < In.rows; ++i)
    {
      for (int j = 0; j  < In.cols; ++j)
      {
        imag(i,j)= (float(pixelPtr[(i*In.cols+ j)*cn+0])*29+float(pixelPtr[(i*In.cols+ j)*cn+1])*150+float(pixelPtr[(i*In.cols+ j)*cn+2])*77)/255; 
      }
    }
  }
  return 0;
}

LWImage<float> Convert_image(const cv::Mat& In)//convert only gray scale image of opencv
{
  unsigned char* pixelPtr = (unsigned char*) In.data;
  int cn=In.channels();
  
  float* data=new float[In.cols*In.rows];

  if (cn==1)//gray scale
  {
    for (int i = 0; i  < In.rows; ++i)
    {
      for (int j = 0; j  < In.cols; ++j)
      {
        data[i*In.cols+ j]= float(pixelPtr[i*In.cols+ j]); 
      }
    }
  }else
  {
    for (int i = 0; i  < In.rows; ++i)
    {
      for (int j = 0; j  < In.cols; ++j)
      {
        data[i*In.cols+ j]= (float(pixelPtr[(i*In.cols+ j)*cn+0])*29+float(pixelPtr[(i*In.cols+ j)*cn+1])*150+float(pixelPtr[(i*In.cols+ j)*cn+2])*77)/255; 
      }
    }

  }
  
  return   LWImage<float>(data,In.cols,In.rows);
}

cv::Mat Convert_image(const LWImage<float> & image,int spline)//convert only gray scale image of opencv
{
	cv::Mat In(image.h,image.w,cv::DataType<unsigned char>::type);
	unsigned char* pixelPtr = (unsigned char*) In.data;
	int cn=In.channels();
	for (int i = 0; i  < In.rows; ++i)
	{
		for (int j = 0; j  < In.cols; ++j)
		{
			float out;
			interpolate_spline(image,spline,j+0.5, i+0.5, &out);	
			if (out>255) out=255;
			if (out<0) out =0;
			pixelPtr[i*In.cols+ j]=unsigned char(int(out)); 
		}
	}
	return   In;
}


//========================feature convertions=================================//
int Convert_detectors(const  std::vector<cv::KeyPoint>& feat1,std::vector<keypoint>& F1){
  F1.clear();
  for (std::vector<cv::KeyPoint>::const_iterator it=feat1.begin();it!=feat1.end();it++){
    keypoint key;
    key.x=it->pt.x +0.5;// opencv 2.4.8 mark the first pixel as (0.5,0.5) which should be (0.5,0.5)  precisely
    key.y=it->pt.y +0.5;
    key.angle=it->angle*PI/180;
    while (key.angle>=2*PI)
      key.angle-=2*PI;
    while (key.angle<0)
      key.angle+=2*PI;
    
    key.response= it->response;
	key.scale=it->size/2;
    F1.push_back(key);

   
  }
  return 0;
}

int Convert_features(const  std::vector<cv::KeyPoint>& feat1,std::vector<keypoint>& F1, cv::Mat descriptors1){
  F1.clear();

  float* ptr = (float*) descriptors1.data;
  int i=0;
  for (std::vector<cv::KeyPoint>::const_iterator it=feat1.begin();it!=feat1.end();it++){
    keypoint key;
    key.x=it->pt.x+0.5;// opencv 2.4.8 mark the first pixel as (0.5,0.5) which should be (0.5,0.5)  precisely
    key.y=it->pt.y+0.5;
    key.angle= (it->angle)*PI/180;// angle is inversed in openCV
    key.scale=it->size/2;
	key.response=it->response;

    float* ptrROW = (float*) descriptors1.row(i).data;
    for (int j=0; j<128;j++){
      key.vec[j]=float(ptrROW[j]);
      }
 
    F1.push_back(key);
    i++;
  }
  return 0;
}

int Convert_matches(const std::vector<cv::DMatch>& matches, std::vector<Pair>& matchesPair){
   for (std::vector<cv::DMatch>::const_iterator it=matches.begin();it!=matches.end();it++)
	   matchesPair.push_back(Pair(it->queryIdx,it->trainIdx,it->distance));
return 0;
}

int read_detectors(const std::string& filename ,  std::vector<cv::KeyPoint>& feat){
  std::ifstream file(filename.c_str());
  if (!file.is_open()){
    std::cout<<"error in reading detector files"<<std::endl;
    return -1;
  }
  int size;
  file>>size;
  for (int i=0; i<size;i++){
    float x, y, angle, scale;
    file>>x>>y>>scale>>angle;   


    cv::KeyPoint key(x,y,scale*2,angle*180/PI);
    feat.push_back(key);
  }
}

int read_matches(const std::string& filename , std::vector<cv::DMatch>& matches){
  std::ifstream file(filename.c_str());
  if (!file.is_open()){
    std::cout<<"error in reading matches files"<<std::endl;
    return -1;
  }
  int size;
  file>>size;
  for (int i=0; i<size;i++){
    int  l, r;
    file>>l>>r;   
    cv::DMatch m(l,r,0);
    matches.push_back(m);
  }
}

cv::Mat convert(libNumerics::matrix<double> M){
	cv::Mat mat(M.nrow(),M.ncol(),cv::DataType<double>::type);
	double* ptr=(double*) mat.data;
	for (int i= 0 ; i < mat.rows;i++)
		for(int j=0; j< mat.cols;j++)
			ptr[i*mat.cols+j]=M(i,j);
	return mat;
}
libNumerics::matrix<double> convert(cv::Mat M)
{
	libNumerics::matrix<double> out(M.rows, M.cols);
	for (int i= 0 ; i < M.rows;i++)
		for(int j=0; j< M.cols;j++)
			out(i,j) = M.at<double>(i,j);
	return out;
}
