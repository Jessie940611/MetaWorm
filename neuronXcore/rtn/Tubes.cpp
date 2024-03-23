// ======================================================================== //
// Copyright 2018-2020 The Contributors                                     //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "CurveUtils.h"
#include "TubesUtils.h"
#include <cstddef>
#include <fstream>
#include <sstream>

namespace tubes {
  using namespace owl;
  using namespace owl::common;

#if _WIN32
  inline double drand48() { return double(rand()) / RAND_MAX; }
#endif

  std::string getExt(const std::string &fileName)
  {
    int pos = fileName.rfind('.');
    if (pos == fileName.npos)
      return "";
    return fileName.substr(pos);
  }

  vec3f randomDirection()
  {
    while (1) {
      vec3f v((float)drand48() * 2.f - 1.f,
              (float)drand48() * 2.f - 1.f,
              (float)drand48() * 2.f - 1.f);
      if (dot(v,v) <= 1.f) return normalize(v);
    }
  }
    
  // try parsing a 'procedural://' pseudo-url specifier:
  Tubes::SP tryProcedural(const std::string &pseudoURL)
  {
    int numLinks = 1000;
    vec2f lengthInterval = vec2f(0.5f,3.5f);
    vec2f radiusInterval = vec2f(0.2f, 0.02f);
    
    int numRecognized
      = sscanf(pseudoURL.c_str(),"procedural://%i:%f,%f:%f,%f",
               &numLinks,
               &lengthInterval.x,&lengthInterval.y,
               &radiusInterval.x,&radiusInterval.y);
    PING;
    PRINT(numRecognized);
    if (numRecognized == 1 || numRecognized == 3 || numRecognized == 5)
      return Tubes::procedural(numLinks,lengthInterval,radiusInterval);
    
    return nullptr;
  }
  

  /*! try '.clusters' file format - will reject if file does not end with
    ".slbin" extension

    clusters file format is what's written by clusterSegmenmts tools,
    and so far is ONLY PROTOTYPICAL

    Format as follows:

    FILE = { 
    int32 numClusters;
    Cluster[numClusters];
    }
   
    Cluster = {
    linear3f xfm;
    int32 numSegments;
    Segments[numSegments];
    }

    Segment = {
    vec3f A   
    vec3f B
    int   trackID
    int   segID;
    }

    currently this parser will only keep the N largest ones, for
    debugging
  */
  // Tubes::SP tryClusters(const std::string &fileName)
  // {
  //   const std::string ext = getExt(fileName);
  //   if (ext != ".clusters")
  //     return nullptr;

  //   std::cout << "recognized 'clusters' file format" << std::endl;
  //   struct Segment {
  //     vec3f A, B;
  //     int trackID,segID;
  //   };

  //   Tubes::SP tubes = std::make_shared<Tubes>();
    
  //   std::ifstream in(fileName,std::ios::binary);

  //   // read N tracks
  //   int numClusters;
  //   in.read((char*)&numClusters,sizeof(numClusters));
  //   PRINT(numClusters);
  //   for (int clusterID=0;clusterID<numClusters;clusterID++) {
  //     linear3f toLocal;
  //     in.read((char*)&toLocal,sizeof(toLocal));
  //     int numSegs;
  //     in.read((char*)&numSegs,sizeof(numSegs));
  //     std::cout << "cluster " << clusterID << " : " << numSegs << " segments" << std::endl;
  //     std::vector<Segment> segments;
  //     for (int segID=0;segID<numSegs;segID++) {
  //       Segment seg;
  //       in.read((char*)&seg,sizeof(seg));
  //       segments.push_back(seg);
  //     }
  //     // if (segments.size() == 1)
  //     {
  //       for (auto seg : segments) {
  //         Tubes::Link link;
  //         link.pos    = seg.A;
  //         link.col    = unsigned(-1);
  //         link.rad    = 1.f;
  //         link.prev   = -1;
  //         tubes->links.push_back(link);
  //         link.prev = tubes->links.size()-1;
  //         link.pos = seg.B;
  //         tubes->links.push_back(link);

  //         // if (segments.size() > 200) {
  //         //   PING;
  //         //   PRINT(seg.A);
  //         //   PRINT(seg.B);
  //         // }
  //       }
  //     }
  //     // PRINT(tubes->links.size());
  //   }

  //   PING; PRINT(tubes->links.size());
  //   // no timplemneted
  //   return tubes;
  // }


  
  /*! try '.slbin' file format - will reject if file does not end with
    ".slbin" extension

    the SLBin file format encodes a list of 'tracks' or stream lines, as follows:

    FILE = { 
    int numTracks; 
    Track[numTracks]; 
    int numPoints; 
    vec3f point[numPoints];
    }

    with Track = {
    int begin;
    int count;
    }

    where 'begin' is a offset in the points[] array, and 'count' is the
    number of control points (starting at that offset) that go into the
    given track. (ie, for individual cylinders 'count' would always be
    '2', for anything longer than two we get a 'track' where
    points[offset] is the start point, and each following of the count
    points coonects to its predecessor.

  */
  Tubes::SP trySLBin(const std::string &fileName,
                     float radius)
  {
#if 1
    throw std::runtime_error("not yet ported....");
#else
    const std::string ext = getExt(fileName);
    if (ext != ".slbin")
      return nullptr;

    const char *subSampleEnv = getenv("TUBES_SUBSAMPLE");
    const int subSample
      = subSampleEnv
      ? atoi(subSampleEnv)
      : 1;
    
    Tubes::SP tubes = std::make_shared<Tubes>();
    tubes->radius = radius;
    std::ifstream in(fileName,std::ios::binary);

    // read N tracks
    int numTracks;
    in.read((char*)&numTracks,sizeof(numTracks));
    std::vector<std::pair<int,int>> tracks(numTracks);
    in.read((char*)tracks.data(),tracks.size()*sizeof(tracks[0]));
      
    // read N points
    int numPoints;
    in.read((char*)&numPoints,sizeof(numPoints));
    std::vector<vec3f> points(numPoints);
    in.read((char*)points.data(),points.size()*sizeof(points[0]));

    if (radius <= 0.f) {
      radius = .1f;
      std::cout << "#rtx-neurons.slbin: no (or invalid) radius specified, defaulting to " << radius << std::endl;
    }
    int numSegments = 0;
    int trackID = 0;
    for (auto track : tracks) {
      int begin = track.first;
      int count = track.second;
      if (count == 1)
        continue;

      bool useThese = drand48()*subSample < 1.f;
      for (int i=1;i<count;i++) {
        Tubes::Link link;
        link.pos0    = points[begin+i-1];
        link.pos1    = points[begin+i];
        link.col    = unsigned(-1);
        link.rad0    = radius;
        link.rad1    = radius;
        if (useThese)
          tubes->links.push_back(link);
      }
      if (useThese)
        numSegments += (count-1);
      trackID++;
    }
    std::cout << OWL_TERMINAL_GREEN
              << "#rtx-neurons: loaded .slbin file with "
              << prettyNumber(tracks.size()) << " stream lines with "
              << prettyNumber(numSegments) << " segments and "
              << prettyNumber(numPoints) << " points/links"
              << OWL_TERMINAL_DEFAULT
              << std::endl;

    // no timplemneted
    return tubes;
#endif
  }


  Tubes::SP tryBCC(const std::string &fileName, float radius)
  {
    const std::string ext = getExt(fileName);
    if (ext != ".bcc")
      return nullptr;

    Tubes::SP tubes = std::make_shared<Tubes>();
    tubes->radius = radius;
    std::ifstream in(fileName,std::ios::binary);

    std::vector<BSpline> curves;
#if 0
    // Some dummies for now
    curves.push_back({{{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.5f}, {2.0f, 0.5f, 0.0f}, {3.0f, 0.0f, 0.0f}}});
    curves.push_back({{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {2.0f, 2.0f, 0.0f}, {3.0f, 3.0f, 0.0f}}});
    curves.push_back({{{0.0f, 0.0f, 0.0f}, {0.0f, 20.0f, 1.0f}, {0.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 3.0f}}});
    curves.push_back({{{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 2.0f}, {0.0f, 0.0f, 3.0f}}});
    curves.push_back({{{-1.0f, 2.0f, 0.0f}, {0.0f, -2.0f, 0.0f}, {1.0f, 4.0f, 0.0f}, {2.0f, -4.0f, 0.0f}}});
    curves.push_back({{{-5.0f, 2.0f, 0.0f}, {0.0f, -0.5f, 1.0f}, {1.0f, 4.0f, 0.0f}, {2.0f, -4.0f, 8.0f}}});
    curves.push_back({{{0.0f, 10.0f, 0.0f}, {0.0f, 11.0f, 11.0f}, {0.0f, -11.0f, 0.0f}, {0.0f, -10.0f, 0.0f}}});
    curves.push_back({{{3.0f, 8.0f, 0.0f}, {0.0f, 15.0f, 11.0f}, {0.0f, -11.0f, 0.0f}, {7.0f, -10.0f, 0.0f}}});
    curves.push_back({{{-10.0f, 20.0f, 0.0f}, {0.0f, -20.0f, 0.0f}, {1.0f, 4.0f, 0.0f}, {2.0f, -4.0f, 0.0f}}});
    curves.push_back({{{0.0f, 20.0f, 0.0f}, {1.0f, 2.0f, 4.5f}, {2.0f, 3.5f, 0.0f}, {3.0f, 4.0f, 0.0f}}});
#endif
    // Ignore any header - these are uniformly parameterized Catmull Rom curves
    in.seekg(8,std::ios::beg);

    uint64_t numCurves = 0;
    in.read((char*)&numCurves, sizeof(numCurves));
    uint64_t numControlPoints = 0;
    in.read((char*)&numControlPoints, sizeof(numControlPoints));
    std::cout << numCurves << ' ' << numControlPoints << '\n';

    // Jump  straight to the loc containing the curves
    in.seekg(64,std::ios::beg);

    curves.resize(numCurves);

    for (size_t i=0; i<numCurves; ++i)
    {
      int32_t numCurvePoints = 0;
      in.read((char*)&numCurvePoints, sizeof(numCurvePoints));
      numCurvePoints = std::abs(numCurvePoints);
      std::vector<vec3f> curvePoints(numCurvePoints);
      in.read((char*)curvePoints.data(), curvePoints.size() * sizeof(vec3f));

      for (size_t j=0;j<numCurvePoints/4; ++j)
      {
        vec3f p0 = curvePoints[j*4];
        vec3f p1 = curvePoints[j*4+1];
        vec3f p2 = curvePoints[j*4+2];
        vec3f p3 = curvePoints[j*4+3];

        float d1 = length(p1-p0);
        float d2 = length(p2-p1);
        float d3 = length(p3-p2);

        vec3f w0 = p1;
        vec3f w1 = (p2 - p0 + 6.f*p1) / 6.f;
        vec3f w2 = (p1 - p3 + 6.f*p2) / 6.f;
        vec3f w3 = p2;
        curves.push_back({{w0,w1,w2,w3}});
      }
    }

    for (auto& curve : curves) {
      tubes->lineStrips.emplace_back();
      tubes->lineStrips.back().resize(4);
      for (int i=0; i<4; ++i) {
        tubes->lineStrips.back()[i].pos = curve[i];
        tubes->lineStrips.back()[i].rad = radius;
        tubes->lineStrips.back()[i].col = unsigned(-1);
      }
    }

    return tubes;
  }
  
  
  Tubes::SP tryPBRT(const std::string &fileName, float radius)
  {
    const std::string ext = getExt(fileName);
    if (ext != ".pbrt")
      return nullptr;

    Tubes::SP tubes = std::make_shared<Tubes>();
    tubes->radius = radius;
    std::ifstream in(fileName,std::ios::binary);

    std::vector<BSpline> curves;

    std::string line;
    while (std::getline(in, line))
    {
      BSpline curve;
      char ignore[256];
      sscanf(line.c_str(), "Shape \"curve\" \"string type\" [ \"cylinder\" ] \"point P\" [ %f %f %f %f %f %f %f %f %f %f %f %f %s",
        &curve[0].x,&curve[0].y,&curve[0].z,
        &curve[1].x,&curve[1].y,&curve[1].z,
        &curve[2].x,&curve[2].y,&curve[2].z,
        &curve[3].x,&curve[3].y,&curve[3].z,
        ignore);
      curves.push_back(curve);
    }

    for (auto& curve : curves) {
      tubes->lineStrips.emplace_back();
      tubes->lineStrips.back().resize(4);
      for (int i=0; i<4; ++i) {
        tubes->lineStrips.back()[i].pos = curve[i];
        tubes->lineStrips.back()[i].rad = radius;
        tubes->lineStrips.back()[i].col = unsigned(-1);
      }
    }

    return tubes;
  }


  /*
    The three dimensional structure of a neuron can be represented in a SWC format (Cannon et al., 1998). SWC is a simple Standardized format. 
    Each line has 7 fields encoding data for a single neuronal compartment:
    an integer number as compartment identifier
    type of neuronal compartment
    0 - undefined
    1 - soma
    2 - axon
    3 - basal dendrite
    4 - apical dendrite
    5 - custom (user-defined preferences)
    6 - unspecified neurites
    7 - glia processes
    x coordinate of the compartment
    y coordinate of the compartment
    z coordinate of the compartment
    radius of the compartment
    parent compartment

    Every compartment has only one parent and the parent compartment for the first point in each file is always -1 (if the file does not include 
    the soma information then the originating point of the tree will be connected to a parent of -1). The index for parent compartments are
    always less than child compartments. Loops and unconnected branches are excluded. All trees should originate from the soma and have parent
    type 1 if the file includes soma information. Soma can be a single point or more than one point. When the soma is encoded as one line in 
    the SWC, it is interpreted as a "sphere". When it is encoded by more than 1 line, it could be a set of tapering cylinders (as in some 
    pyramidal cells) or even a 2D projected contour ("circumference").
  
  */
  //['r','g','b','c','m','y','k','g','DarkGray']  
  unsigned int neuronColorMapping(const unsigned type) 
  {
    vec3f col(0.0);
    // case 0: col = vec3f(255.0/255.0, 137.0 / 255.0, 72.0 / 255.0); break; // doesn't seem to be used
    // case 5: col = vec3f(255.0/255.0, 0.0, 0.0); break; // doesn't seem to be used...
      
    // case 1: col = vec3f(255.0/255.0, 137.0 / 255.0, 72.0 / 255.0); break; // soma

    // // soma white, axon white, dentrites either blue or red
    // switch (type) {
    //   case 1: col = vec3f(1.0f); break; // soma
    //   case 2: col = vec3f(1.0, 1.0, 1.0); break; // axon
    //   case 3: col = vec3f(1.0, 0.0, 0.0); break; // basal dendrite
    //   case 4: col = vec3f(0.0, 0.0, 1.0); break; // apical dendrite
    //   default: col = vec3f(0.0, 0.0, 0.0); break;
    // }

    // soma white, axon white, dentrites either blue or red
    switch (type) {
    case 1: col = vec3f(1.0f); break; // soma
    case 2: col = vec3f(1.0, 1.0, 1.0); break; // axon
    case 3: col = vec3f(156.0 / 255.0, 15.0 / 255.0, 12.0 / 255.0); break; // basal dendrite
    case 4: col = vec3f(10.0 / 255.0, 32.0 / 255.0, 156.0 / 255.0); break; // apical dendrite
    default: col = vec3f(0.0, 0.0, 0.0); break;
    }

    // // soma red, all others blue
    // switch (type) {
    //   case 1: col = vec3f(1.0, 0.0, 0.0); break; // soma
    //   case 2: col = vec3f(0.2, .2, 1.0); break; // axon
    //   case 3: col = vec3f(0.2, .2, 1.0); break; // basal dendrite
    //   case 4: col = vec3f(0.2, .2, 1.0); break; // apical dendrite
    //   default: col = vec3f(0.2, .2, 1.0); break;
    // }

    //   // case 0: col = vec3f(256.0/255.0,231.0/255.0,37.0/255.0); break;
    //   // case 1: col = vec3f(158.0/255.0,217.0/255.0,58.0/255.0); break;
    //   // case 2: col = vec3f(137.0/255.0,213.0/255.0,72.0/255.0); break;
    //   // case 3: col = vec3f(214.0/255.0,210.0/255.0,80.0/255.0); break;
    //   // case 4: col = vec3f(71.0/255.0,193.0/255.0,110.0/255.0); break;
    //   // case 5: col = vec3f(62.0/255.0,73.0/255.0,137.0/255.0); break;
    //   // case 6: col = vec3f(62.0/255.0,73.0/255.0,137.0/255.0); break;
    //   // case 7: col = vec3f(72.0/255.0,29.0/255.0,111.0/255.0); break;
    //   // case 8: col = vec3f(69.0/255.0,5.0/255.0,89.0/255.0); break;
    //   // default: col = vec3f(71.0/255.0,193.0/255.0,110.0/255.0); break;
    //   // case 5: col = vec3f(58.0/255.0,186.0/255.0,118.0/255.0)
    //   // case 6: col = vec3f(35.0/255.0,169.0/255.0,131.0/255.0)
    //   // case 7: col = vec3f(30.0/255.0,157.0/255.0,137.0/255.0)
    //   // case 8: col = vec3f(42.0/255.0,120.0/255.0,142.0/255.0)
    // // case 0: col = vec3f(.85, .17, .19); break;
    // // case 1: col = vec3f(1.0, 0.0, 0.0); break;
    // // case 2: col = vec3f(0.0, 0.0, 1.0); break;
    // // case 3: col = vec3f(0.0, 1.0, 0.0); break;
    // // case 4: col = vec3f(1.0, 0.0, 1.0); break;
    // // case 5: col = vec3f(1.0, 1.0, 0.0); break;
    // // case 6: col = vec3f(0.0, 1.0, 1.0); break;
    // // case 7: col = vec3f(0.2, 0.2, 0.2); break;
    // // case 8: col = vec3f(0.85, 0.05, 0.11); break;
    // // default: col = vec3f(0.86, 0.11, 0.26);
    typedef unsigned char u_char;
    u_char r = (u_char)clamp((u_char) (col.x * 255.f), (u_char)0, (u_char)255);
    u_char g = (u_char)clamp((u_char) (col.y * 255.f), (u_char)0, (u_char)255);
    u_char b = (u_char)clamp((u_char) (col.z * 255.f), (u_char)0, (u_char)255);
    // nvm: don't know why, but white becomes green when "|"ing 255 << 24...
    return (r | (g << 8) | (b << 16)) ;// | (u_char(255) << 24); 
  }

  Tubes::SP trySWC_folder_Method1(const std::string& fileName) 
  {
      const std::string ext = getExt(fileName);
      if (ext.size() < 5)
          return nullptr;

      Tubes::SP tubes = std::make_shared<Tubes>();
      std::string vertexFile = fileName + "vertex";
      std::string indicesFile = fileName + "indices";
      std::string rec_voltagesFile = fileName + "rec_voltages";

      std::ifstream vertexin(vertexFile);
      std::ifstream indicesin(indicesFile);
      std::ifstream rec_voltagesIn(rec_voltagesFile);
      int idx, type, prev;
      vec3f Pos;
      float ra;
      std::string line;
      std::vector<vec3f> positions;
      std::vector<float> radii;
      std::vector<unsigned> colorsID;

      float fmin = 10000.0f; // -47.0f
      float val_fmax = -10000.0f; // -104.858

      const float v_min = -105.0f;
      const float v_max = -17.0f;
      const int voltagesSize = 1998;

      std::vector<std::vector<unsigned>> A;
      tubes->primColorsVec.resize(0);

      std::getline(rec_voltagesIn, line); // Skip the first line.
      while (std::getline(rec_voltagesIn, line))
      {
          if (line.find_first_of("#") != std::string::npos)
              continue;
          std::istringstream iss(line);
          std::vector<unsigned> aa;
          double val;
          for (int i = 0; i < voltagesSize; i++)// To fix later..
          {
              iss >> val;
              if (val > val_fmax)
                  val_fmax = val;
              if (val < fmin)
                  fmin = val;
              unsigned v = (unsigned)( 255.0f * (val - v_min) / 120.0f);
              aa.push_back(v);
          }          
          A.push_back(aa);
      }

      int xSz = A.size();
      int ySz = A[0].size();


      for (int j = 0; j < ySz; j++)      
      {
          std::vector<unsigned> aa;
          aa.resize(0);
          for (int i = 0; i < xSz; i++)
          {
              aa.push_back(rand() * 255 / RAND_MAX);
          }
          tubes->primColorsVec.push_back(aa);
      }
      
      std::cout << OWL_TERMINAL_GREEN
          << "#neurons: loaded .swc-like folder file with "
          << prettyNumber(tubes->primColorsVec[0].size()) << " rec_voltages "
          << prettyNumber(tubes->primColorsVec.size()) << " nodes "
          << std::endl
          << "Max :" << val_fmax << " Min :" << fmin << std::endl
          << OWL_TERMINAL_DEFAULT
          <<std::endl;


      while (std::getline(vertexin, line))
      {
          if (line.find_first_of("#") != std::string::npos)
              continue;
          std::istringstream iss(line);
          iss >> /*idx >> type >>*/ Pos.x >> Pos.y >> Pos.z >> ra >> prev;
          positions.push_back(Pos);
          radii.push_back(ra);
          colorsID.push_back(prev);
      }
      std::cout << OWL_TERMINAL_GREEN
          << "#rtx-neurons: loaded .swc-like folder file with "
          << prettyNumber(positions.size()) << " Points "
          << OWL_TERMINAL_DEFAULT
          << std::endl;
      int cur;
      while (std::getline(indicesin, line))
      {
          if (line.find_first_of("#") != std::string::npos)
              continue;
          std::istringstream iss(line);
          iss >> cur >> prev;
          tubes->lineStrips.emplace_back();
          tubes->lineStrips.back().push_back({ positions[cur],radii[cur],colorsID[cur] });
          tubes->lineStrips.back().push_back({ positions[prev],radii[prev],colorsID[prev] });
      }

      std::cout << OWL_TERMINAL_GREEN
          << "#rtx-neurons: loaded .swc-like folder file with "
          << prettyNumber(tubes->lineStrips.size()) << " stream lines "
          << OWL_TERMINAL_DEFAULT
          << std::endl;
      return tubes;
  }

  Tubes::SP trySWC(const std::string& fileName)
  {
    const std::string ext = getExt(fileName);
    if (ext != ".swc")
      return nullptr;
    Tubes::SP tubes = std::make_shared<Tubes>();
    std::ifstream in(fileName);
    int idx, type,prev;
    vec3f Pos;
    float ra; 
    std::string line;
    std::vector<vec3f> positions;
    std::vector<float> radii;
    while(std::getline(in, line))
      {   
        if (line.find_first_of("#") != std::string::npos)
          continue;
        std::istringstream iss(line);    
        iss >> idx >> type >> Pos.x >> Pos.y >> Pos.z >> ra >> prev;
        positions.push_back(Pos);
        radii.push_back(ra);
        if (prev < 0) continue;
        tubes->lineStrips.emplace_back();
        tubes->lineStrips.back().push_back({Pos,ra,neuronColorMapping(type)});
        tubes->lineStrips.back().push_back({positions[prev-1],radii[prev-1],neuronColorMapping(type)});
      }   
    std::cout << OWL_TERMINAL_GREEN
              << "#rtx-neurons: loaded .swc file with "
              << prettyNumber(tubes->lineStrips.size()) << " stream lines "
      // << prettyNumber(numSegments) << " segments and "
      // << prettyNumber(numPoints) << " points/links"
              << OWL_TERMINAL_DEFAULT
              << std::endl;
    // no timplemneted
    return tubes;
  }

  /*
    this only for the polyline style obj. not the triangle obj file 

  */

  Tubes::SP tryOBJ(const std::string& fileName,float radius = 0.00005)
  {
    const std::string ext = getExt(fileName);
    if (ext != ".obj")
      return nullptr;
    Tubes::SP tubes = std::make_shared<Tubes>();
    // Tubes::SP tubesTmp= std::make_shared<Tubes>();
    std::ifstream in(fileName);

    int idx, type, prev;
    vec3f Pos;
    float ra;
    std::string line;
      
    // std::vector<std::pair<int, int>> lines;
    char Ty;

    std::vector<vec3f> positions;
    std::vector<unsigned>   colors;
    // the previous line segment to build up a linked list over segments
    std::pair<int,int> prevSegment = { -1, -1 };
    std::size_t segmentCount = 0;
    
    tubes->radius = radius;
    while (std::getline(in, line)) {
      if (line.find_first_of("#") != std::string::npos)
        continue;
      if (line.find_first_of("O") != std::string::npos)
        continue;
      std::istringstream iss(line);
      if (line.find_first_of("v") != std::string::npos) {
        vec3f pt;
        iss >> Ty >> pt.x >> pt.y >> pt.z;
        positions.push_back(pt);
        if (!iss.eof()) {
          vec3f col;
          iss >> col.x >> col.y >> col.z;
          unsigned r=(unsigned)clamp(col.x*255.f,0.f,255.f);
          unsigned g=(unsigned)clamp(col.y*255.f,0.f,255.f);
          unsigned b=(unsigned)clamp(col.z*255.f,0.f,255.f);
          colors.push_back(r | (g<<8) | (b<<16) | (255<<24));
        } else {
          colors.push_back(unsigned(-1));
        }
      }
      else if (line[0] == 'l') {
        std::pair<int, int> segment;
        iss >> Ty >> segment.first >> segment.second;
        segment.first --;
        segment.second --;
        if (segment.first >= positions.size())
          continue;
        if (segment.second >= positions.size())
          continue;
        segmentCount++;
        // assert(segment.first >= 0);
        // assert(segment.first < tubes->links.size());
        // assert(segment.second >= 0);
        // assert(segment.second < tubes->links.size());
        // tubes->links[segment.second].prev = segment.first;
        if (prevSegment.second != segment.first) {
          tubes->lineStrips.emplace_back();
          tubes->lineStrips.back().push_back({positions[segment.first],tubes->radius,colors[segment.first]});
        }
        tubes->lineStrips.back().push_back({positions[segment.second],tubes->radius,colors[segment.second]});
        prevSegment = segment;
      }
    }

    std::cout << OWL_TERMINAL_GREEN
              << "#rtx-neurons: loaded .obj file with "
               << prettyNumber(tubes->lineStrips.size()) << " stream lines "
              // << prettyNumber(lines.size()) << " segments and "
              << segmentCount << " points/links"
              << OWL_TERMINAL_DEFAULT
              << std::endl;
    // no timplemneted
    return tubes;
  }

  /*! load a file - supports various file formats of procedural
    methods of generating, see below:

    filename= procedural://num:a,b:c,d: returns
    Tubes::prodedural(num,{a,b},{c,d}. This will *IGNORE* teh
    'radius' parameter

    filename = *.slbin -> use slbin reader
  */
  Tubes::SP Tubes::load(const std::string& fileName,
                        float radius)
  {
      if (Tubes::SP tubes = trySWC_folder_Method1(fileName)) {
          if (radius > 0.f) tubes->radius = radius;
          return tubes;
      }
    // try polyline "obj"
    if (Tubes::SP tubes = tryOBJ(fileName,radius))
      return tubes;

    // try '*.pbrt'
    if (Tubes::SP tubes = tryPBRT(fileName,radius))
      return tubes;

    // try '*.swc'
    if (Tubes::SP tubes = trySWC(fileName)) {
      if (radius > 0.f) tubes->radius = radius;
      return tubes;
    }

    // try '*.clusters'
    // if (Tubes::SP tubes = tryClusters(fileName)) {
    //   if (radius > 0.f) tubes->radius = radius;
    //   return tubes;
    // }
      
    //if (Tubes::SP tubes = trySLTxt(fileName))
    //      return tubes;

    // try '*.slbin'
    if (Tubes::SP tubes = trySLBin(fileName,radius))
      return tubes;

    // try '*.bcc'
    if (Tubes::SP tubes = tryBCC(fileName,radius))
      return tubes;

        // try 'procedural://' specifier
    if (Tubes::SP tubes = tryProcedural(fileName))
      return tubes;
    
    throw std::runtime_error("could not load/create input '"+fileName+"'");
#if 0
    // this is code for random *particle* generation
    float radius = 1.f;
    Tubes::SP tubes = std::make_shared<Tubes>();
    int numParticles = 200;
    if (sscanf(fileName.c_str(), "%i.random", &numParticles))
      std::cout << "using " << numParticles << " as instructured on cmdline" << std::endl;

    float size = powf(numParticles, 1.f / 3.f);

    for (int i = 0; i < numParticles; i++) {
      vec3f pos = size * 2.f * vec3f(drand48(), drand48(), drand48());
      Link comp = { pos,radius,-1 };
      
      // comp.r = .01f+.08f*drand48();
      tubes->links.push_back(comp);
    }
    tubes->radius = radius;
    return tubes;
#endif
  }

  /*! load several files */
  Tubes::SP Tubes::load(const std::vector<std::string>& fileNames,
                        float radius)
  {
    Tubes::SP result = nullptr;
    for (std::size_t i=0; i<fileNames.size(); ++i) {
      Tubes::SP tubes = load(fileNames[i],radius);
      if (tubes != nullptr) {
        if (result != nullptr) {
          // for (std::size_t j=0; j<tubes->links.size(); ++j) {
          //   // Yeah, I'm sure that's not the most elegant way but should
          //   // ensure that the links are correctly set up even for file
          //   // types that I can't test..
          //   if (tubes->links[j].prev >= 0)
          //     tubes->links[j].prev += (int)result->links.size();
          // }
          result->lineStrips.insert(result->lineStrips.end(),tubes->lineStrips.begin(),tubes->lineStrips.end());
        } else {
          result = tubes;
        }
      } else {
        throw std::runtime_error("could not load/create input from file no. '"+std::to_string(i)+"'");
      }
    }
    return result;
  }

  /*! procedurally generate some linked tubes,so we have "something"
    to test.  numTubes --- maximum supported size on 1080 card:
    29000000.  lengthInterval radiusInterval
  */
  Tubes::SP Tubes::procedural(int numTubes,
                              vec2f lengthInterval,
                              vec2f radiusInterval)
  {
#if 1
    throw std::runtime_error("not yet ported....");
#else
    PING;
    PRINT(lengthInterval);
    PRINT(radiusInterval);
    
    Tubes::SP tubes = std::make_shared<Tubes>();
    float size = powf((float)numTubes, 1.f / 3.f);
    Link firstNode;
    firstNode.pos = size * vec3f((float)drand48(),
                                 (float)drand48(),
                                 (float)drand48());
    firstNode.rad = radiusInterval.x;
    firstNode.prev = -1;
    tubes->links.push_back(firstNode);

    for (int i = 0; i < numTubes; i++) {
      Link comp;
      comp.prev = i;
      float randomLen
        = lengthInterval.x + ((float)lengthInterval.y - lengthInterval.x) * drand48();
      float randomRadius
        = tubes->links[i].rad
        + radiusInterval.y * ((float)drand48() * 2.f - 1.f );
    
      comp.pos
        = tubes->links[i].pos
        + randomDirection() * randomLen;
      comp.rad = std::max(.1f*radiusInterval.x,fabsf(randomRadius));
      
      tubes->links.push_back(comp);
    }
    std::cout << "generated " << numTubes << " tubes as instructured" << std::endl;
    return tubes;
#endif
  }

  Tubes::HostGeom Tubes::asLinks() const
  {
    Tubes::HostGeom result;

    for (const LineStrip &ls : lineStrips) {
      for (ptrdiff_t i=0; i<ptrdiff_t(ls.size())-1; ++i) {
        Vertex v0 = ls[i];
        Vertex v1 = ls[i+1];
        result.vertices.push_back(v0.pos);
        result.vertices.push_back(v1.pos);
        result.endPointRadii.push_back(v0.rad);
        result.endPointRadii.push_back(v1.rad);
        result.primColors.push_back(v0.col);
      }
    }

    return result;
  }

  Tubes::HostGeom Tubes::asCubicSplines() const
  {
    Tubes::HostGeom result;

    for (const LineStrip &ls : lineStrips) {
      for (size_t i=0; i<ls.size(); i += 3) {
        Vertex v0 = ls[i];
        Vertex v1 = ls[std::min(ptrdiff_t(i)+1,ptrdiff_t(ls.size())-1)];
        Vertex v2 = ls[std::min(ptrdiff_t(i)+2,ptrdiff_t(ls.size())-1)];
        Vertex v3 = ls[std::min(ptrdiff_t(i)+3,ptrdiff_t(ls.size())-1)];
        result.vertices.push_back(v0.pos);
        result.vertices.push_back(v1.pos);
        result.vertices.push_back(v2.pos);
        result.vertices.push_back(v3.pos);
        result.endPointRadii.push_back(v0.rad);
        result.endPointRadii.push_back(v3.rad);
        result.primColors.push_back(v0.col);

        // Make sure when we're at the end of the
        // curve we don't pad that last control
        // point in the loop with three additional,
        // degenerate points
        if (i+4 == ls.size())
          break;
      }
    }

    std::cout << "num curves (before split): " << result.vertices.size()/4 << '\n';

    // Reshetov splits into 8 curves (might be too much for us)
    if (curveSegments == 2)
      result = splitBSplineLinks<2>(result);
    else if (curveSegments == 4)
      result = splitBSplineLinks<4>(result);
    else if (curveSegments == 8)
      result = splitBSplineLinks<8>(result);
    else if (curveSegments != 1)
      throw std::runtime_error("invalid --curve-segments, must be 1,2,4, or 8!");

    std::cout << "num curves (after split) : " << result.vertices.size()/4 << '\n';

    return result;
  }

  /*! returns world space bounding box of the scene; to be used for
    the viewer to automatically set camera and motion speed */
  box3f Tubes::getBounds() const
  {
    box3f bounds;
    for (const LineStrip &ls : lineStrips) {
      for (const Vertex &v : ls) {
        bounds.extend(v.pos);
      }
    }
    PRINT(bounds);
    return box3f(bounds.lower - radius, bounds.upper + radius);
  }

}
