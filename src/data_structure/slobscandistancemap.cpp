/*
 * Copyright (c) 2018 Kay Gawlik <kaydev@amarunet.de> <kay.gawlik@beuth-hochschule.de> <kay.gawlik@charite.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "slobscandistancemap.h"


#define _USE_MATH_DEFINES

#include<map>
#include<limits>
#include<cmath>

#include<opencv/cv.hpp>

#include<octdata/datastruct/series.h>
#include<octdata/datastruct/bscan.h>
#include<octdata/datastruct/sloimage.h>


#include<data_structure/matrx.h>
#include<data_structure/point2d.h>
#include <helper/slocoordtranslator.h>


namespace
{

	typedef double DistanceType;

	class TrailMap
	{
		std::vector<PixtureElement> actTrailDist;
		std::vector<PixtureElement> nextTrailDist;
		DistanceType actDist = 0;
	public:
		void clear()
		{
			actTrailDist.clear();
			nextTrailDist.clear();
			actDist = 0;
		}

		void emplace(DistanceType dist, const PixtureElement& ele)
		{
			if(dist == actDist)
				actTrailDist.push_back(ele);
			else
				nextTrailDist.push_back(ele);
		}

		void getElement(PixtureElement& ele, DistanceType& dist)
		{
			if(actTrailDist.size() == 0)
			{
				actTrailDist.swap(nextTrailDist);
				actDist += 1;
			}
			dist = actDist;
			ele  = actTrailDist.back();
			actTrailDist.pop_back();
		}
		std::size_t size() const { return actTrailDist.size() + nextTrailDist.size(); }
	};


	struct SlideInfo
	{
		SlideInfo() = default;

		SlideInfo(DistanceType distance, std::size_t bscanId, std::size_t ascanId)
		: distance(distance)
		, bscanId (bscanId)
		, ascanId (ascanId)
		{}

		bool operator<(const SlideInfo& info)                   const { return distance < info.distance; }
		operator bool()                                         const { return distance != std::numeric_limits<DistanceType>::infinity(); }


		DistanceType distance = std::numeric_limits<DistanceType>::infinity();
		std::size_t bscanId   = std::numeric_limits<std::size_t>::max();
		std::size_t ascanId   = std::numeric_limits<std::size_t>::max();
	};


	class PixelInfo
	{
	public:
		enum class Status : uint8_t { FAR_AWAY, ACCEPTED, BRODER };

		Status status = Status::FAR_AWAY;
		std::size_t ascan = 0;
		bool hasValue = false;

		SlideInfo val1;
		SlideInfo val2;

		bool updateValue(const SlideInfo& info)
		{
			if(val2 < info)
				return false;

			val2 = info;
			if(val2 < val1)
				std::swap(val1, val2);

			hasValue = true;
			return true;
		}
	};


	class FillPreCalcData
	{
		SloBScanDistanceMap::PreCalcDataMatrix& matrix;
		const OctData::Series& series;

		typedef Matrix<PixelInfo> PixelMap;

		PixelMap pixelMap;
		TrailMap trailMap;

		SloCoordTranslator transformCoord;

		constexpr static const DistanceType maxDistance = 25;
		std::size_t actBscanNr = 0;

		static Point2D coordSLO2Point(const OctData::CoordSLOpx& c)                { return Point2D(c.getXf(), c.getYf()); }

		class ValueSetter
		{
			FillPreCalcData& ctm;
			bool add2TrailMap = false; // add2TrailMap == false: For set broder value on each a-scan position to stop evaluation from other b-scans on this bariere (reduce calculation time and artefacts)
			std::size_t nextAscan = 0;
		public:
			ValueSetter(FillPreCalcData& ctm, bool add2TrailMap) : ctm(ctm), add2TrailMap(add2TrailMap) {}

			void operator()(const OctData::CoordSLOpx& coord, std::size_t ascan)
			{
				const std::size_t x = static_cast<std::size_t>(coord.getX());
				const std::size_t y = static_cast<std::size_t>(coord.getY());

				if(x >= ctm.pixelMap.getSizeX() || y >= ctm.pixelMap.getSizeY())
					return;

				PixelInfo& info = ctm.pixelMap(x, y);
				info.status = PixelInfo::Status::BRODER;
				info.ascan  = ascan;
				if(add2TrailMap)
				{
					info.updateValue(SlideInfo(0, ctm.actBscanNr, ascan));
					ctm.trailMap.emplace(0, PixtureElement(x, y));
				}
			}
			constexpr static const bool calcDistMap = false;

			void loopInit()                          { nextAscan = 0; }
			bool loopIncrement(std::size_t maxAscan) { ++nextAscan; return nextAscan < maxAscan; }
			std::size_t getNextAscanNr() const       { return nextAscan; }
		};

		class FindMinAScan
		{
			OctData::CoordSLOpx pos;
			std::size_t minAscan = 0;
			double minDistance = std::numeric_limits<double>::infinity();

			std::size_t nextAscan = 0;
			double lastDistance1 = std::numeric_limits<double>::infinity();
			double lastDistance2 = std::numeric_limits<double>::infinity();
			enum class State { Start, Init, TestPost, Pos, Neg, End };
			State state;

		public:
			FindMinAScan(const OctData::CoordSLOpx& pos, std::size_t startAScan) : pos(pos), minAscan(startAScan) {}

			void operator()(const OctData::CoordSLOpx& coord, std::size_t ascan)
			{
				double d = coord.absQuad(pos);
				if(d < minDistance)
				{
					minDistance = d;
					minAscan    = ascan;
				}
				lastDistance2 = lastDistance1;
				lastDistance1 = d;
			}

			std::size_t getMinAScan() const { return minAscan; }
			double getMinDistance()   const { return std::sqrt(minDistance); }

			void loopInit()                          { nextAscan = minAscan; state = State::Start; }
			bool loopIncrement(std::size_t maxAscan)
			{
				if(state == State::Pos || state == State::Neg)
					if(lastDistance1 > lastDistance2)
						return false;

				switch(state)
				{
					case State::Start: // iteration 0, no value has checked
						state = State::Init;
						return true;
					case State::Init: // iteration 1, the start value has checked
						if(minAscan == maxAscan-1) { state = State::Neg     ; --nextAscan; } // end of bscan, only neg is possible
						else                       { state = State::TestPost; ++nextAscan; }
						return true;
					case State::TestPost:
						if(lastDistance1 > lastDistance2) { state = State::Neg; --nextAscan; } // wrong direction
						else                              { state = State::Pos; ++nextAscan; } // pos is good
						return true;
					case State::Pos:
						++nextAscan;
						return nextAscan < maxAscan;
					case State::Neg:
						return nextAscan-- > 0;
					case State::End:
						return false;
				}
				return false;
			}
			std::size_t getNextAscanNr() const       { return nextAscan; }
		};



		template<typename AScanHandler>
		void addLineScan(const OctData::BScan& bscan, AScanHandler& handler)
		{
			const OctData::CoordSLOpx start_px = transformCoord(bscan.getStart());
			const OctData::CoordSLOpx   end_px = transformCoord(bscan.getEnd()  );

			const std::size_t bscanWidth = static_cast<std::size_t>(bscan.getWidth());

			if(bscanWidth < 2)
				return;

// 			for(std::size_t i=0; i<bscanWidth; ++i)
			handler.loopInit();
			while(handler.loopIncrement(bscanWidth))
			{
				const std::size_t i = handler.getNextAscanNr();
				const double v = static_cast<double>(i)/static_cast<double>(bscanWidth-1);
				const OctData::CoordSLOpx actPos = start_px*(1-v) + end_px*v;
				handler(actPos, i);
			}
		}


		template<typename AScanHandler>
		void addCircleScan(const OctData::BScan& bscan, AScanHandler& handler)
		{
			const OctData::CoordSLOpx start_px  = transformCoord(bscan.getStart ());
			const OctData::CoordSLOpx center_px = transformCoord(bscan.getCenter());

			const std::size_t bscanWidth = static_cast<std::size_t>(bscan.getWidth());

			if(bscanWidth < 2)
				return;

			bool clockwise = bscan.getClockwiseRot();

			double radius = start_px.abs(center_px);
			double ratio  = start_px.getXf() - center_px.getXf();
			double nullAngle = acos( ratio/radius )/M_PI/2.;

			const int rotationFactor = clockwise?1:-1;

			double mX = center_px.getXf();
			double mY = center_px.getYf();

// 			for(std::size_t i=0; i<bscanWidth; ++i)
			handler.loopInit();
			while(handler.loopIncrement(bscanWidth))
			{
				const std::size_t i = handler.getNextAscanNr();
				const double v     = static_cast<double>(i)/static_cast<double>(bscanWidth-1);
				const double angle = (v+nullAngle)*2.*M_PI*rotationFactor;

				const double x = cos(angle)*radius + mX;
				const double y = sin(angle)*radius + mY;
				const OctData::CoordSLOpx actPos(x, y);
				handler(actPos, i);
			}
		}


		template<typename AScanHandler>
		void addBScan(const OctData::BScan& bscan, AScanHandler& pixelSetter)
		{
			switch(bscan.getBScanType())
			{
				case OctData::BScan::BScanType::Line:
					addLineScan(bscan, pixelSetter);
					break;
				case OctData::BScan::BScanType::Circle:
					addCircleScan(bscan, pixelSetter);
					break;
				case OctData::BScan::BScanType::Unknown:
					break;
			}
		}


		template<typename AScanHandler>
		void addBScans(AScanHandler& pixelSetter, bool evaluation)
		{
			std::size_t bscanNr = 0;
			for(const OctData::BScan* bscan : series.getBScans())
			{
				actBscanNr = bscanNr;
				if(bscan)
					addBScan(*bscan, pixelSetter);

				if(evaluation)
					calcActDistMap();

				++bscanNr;
			}
		}

		// ----------------------
		// create L1 distance map
		// ----------------------
		inline void addTrail(std::size_t x, std::size_t y, DistanceType distance, PixelInfo& info, std::size_t ascan)
		{
			info.status = PixelInfo::Status::ACCEPTED;
			info.ascan  = ascan;

			if(info.updateValue(SlideInfo(distance, actBscanNr, ascan)))
				trailMap.emplace(distance, PixtureElement(x, y));
		}


		inline void calcSetTrailDistanceL1(std::size_t x, std::size_t y, DistanceType distance, std::size_t ascan)
		{
			PixelInfo& newTrailInfo = pixelMap(x, y);
			if(newTrailInfo.status == PixelInfo::Status::FAR_AWAY)
			{
				DistanceType trailDistance = distance + 1;
				if(trailDistance < maxDistance)
					addTrail(x, y, trailDistance, newTrailInfo, ascan);
			}
		}

		void calcActDistMap()
		{
			DistanceType distance    = 0;

			while(trailMap.size() > 0 && distance < maxDistance)
			{
				PixtureElement aktEle;
				trailMap.getElement(aktEle, distance);

				const std::size_t aktX = aktEle.getX();
				const std::size_t aktY = aktEle.getY();

				std::size_t ascan = pixelMap(aktX, aktY).ascan;

				if(aktY > 0                    ) calcSetTrailDistanceL1(aktX  , aktY-1, distance, ascan);
				if(aktY < pixelMap.getSizeY()-1) calcSetTrailDistanceL1(aktX  , aktY+1, distance, ascan);
				if(aktX > 0                    ) calcSetTrailDistanceL1(aktX-1, aktY  , distance, ascan);
				if(aktX < pixelMap.getSizeX()-1) calcSetTrailDistanceL1(aktX+1, aktY  , distance, ascan);
			}

			trailMap.clear();

			for(PixelInfo& info : pixelMap) // reset map for next b-scan calculation
			{
				if(info.status != PixelInfo::Status::BRODER)
					info.status = PixelInfo::Status::FAR_AWAY;
			}
		}

		// ---------------------
		// fill broder functions
		// ---------------------
		double calcIntersectionX(const Point2D& lowerPoint, const Point2D& upperPoint, double y)
		{
			const double lengthY = upperPoint.getY() - lowerPoint.getY();
			const double lengthX = upperPoint.getX() - lowerPoint.getX();
			const double intersectionPosY = y - lowerPoint.getY();
			const double intersectionFrac = intersectionPosY/lengthY;
			const double intersectionX = intersectionFrac*lengthX + lowerPoint.getX();
			return intersectionX;
		}

		bool setIntersections(const Point2D& lowerPoint, const Point2D& upperPoint, double y, double& pos1, double& pos2)
		{
			double pos = calcIntersectionX(lowerPoint, upperPoint, y);
			if(pos1 == std::numeric_limits<double>::infinity())
			{
				pos1 = pos;
				return false;
			}
			pos2 = pos;
			return true;
		}

		void drawScanLine(std::size_t y, std::size_t x1, std::size_t x2)
		{
			      PixelInfo*       it    = pixelMap.scanLine(y);
			const PixelInfo* const itEnd = pixelMap.scanLine(y) + x2;
			it += x1;
			for(;it != itEnd; ++it)
				it->status = PixelInfo::Status::BRODER;
		}

		void fillConvexBroder(const OctData::Series::BScanSLOCoordList& broderPoints)
		{
			if(broderPoints.size() < 4)
				return;

			// scan line algo
			const std::size_t sizeY = pixelMap.getSizeY();
			const std::size_t sizeX = pixelMap.getSizeX();

			for(std::size_t y = 0; y < sizeY; ++y)
			{
// 				const double yd = static_cast<double>(y);
				Point2D::value_type yd = static_cast<Point2D::value_type>(y);
				double pos1 = std::numeric_limits<double>::infinity();
				double pos2 = std::numeric_limits<double>::infinity();
				Point2D lastPoint = coordSLO2Point(transformCoord(*(broderPoints.end()-1)));
				for(const OctData::CoordSLOmm& actPoint : broderPoints)
				{
					Point2D actPointPx = coordSLO2Point(transformCoord(actPoint));
					     if(lastPoint .getY() < yd && actPointPx.getY() >= yd) { if(setIntersections(lastPoint, actPointPx , yd, pos1, pos2)) break; }
					else if(actPointPx.getY() < yd && lastPoint .getY() >= yd) { if(setIntersections(actPointPx , lastPoint, yd, pos1, pos2)) break; }
					lastPoint = actPointPx;
				}

				if(pos2 < pos1)
					std::swap(pos1, pos2);

				if(pos1 != std::numeric_limits<double>::infinity())
				{
					pos1 -= 1;
					pos2 += 2;
					if(pos1 > 0                           ) drawScanLine(y, 0                                           , std::min(sizeX, static_cast<std::size_t>(pos1)));
					if(pos2 < static_cast<double>(sizeY-1)) drawScanLine(y, static_cast<std::size_t>(std::max(0., pos2)), sizeX                                          );
				}
				else
					drawScanLine(y, 0, sizeX);
			}

		}


		void creatL1DistanceMap()
		{
			const OctData::Series::BScanSLOCoordList& convexHull = series.getConvexHull();
			const OctData::SloImage& sloImage = series.getSloImage();

			cv::Mat sloImageMat = sloImage.getImage();
			if(sloImageMat.empty())
				return;

			fillConvexBroder(convexHull);

			ValueSetter ivs(*this, false);
			addBScans(ivs, false);

			ValueSetter apvs(*this, true);
			addBScans(apvs, true);

		}




		// --------------------
		// finisch distance map
		// --------------------

		void recalcDistDataL2(std::size_t x, std::size_t y, SloBScanDistanceMap::InfoBScanDist& info, const SlideInfo& slideInfo)
		{
			std::size_t bscanNr = info.bscan;
			if(bscanNr >= series.bscanCount())
				return;

			FindMinAScan fmas(OctData::CoordSLOpx(static_cast<double>(x), static_cast<double>(y)), slideInfo.ascanId);

			const OctData::BScan* bscan = series.getBScan(bscanNr);
			if(bscan)
				addBScan(*bscan, fmas);

			info.ascan    = fmas.getMinAScan();
			info.distance = fmas.getMinDistance();
		}

		void fillPreCalcData()
		{
			SloBScanDistanceMap::PreCalcDataMatrix::value_type* itOut = matrix.begin();
			PixelMap::value_type* itIn = pixelMap.begin();

			const std::size_t sizeX = matrix.getSizeX();
			const std::size_t sizeY = matrix.getSizeY();

			for(std::size_t y = 0; y < sizeY; ++y)
			{
				for(std::size_t x = 0; x < sizeX; ++x)
				{
					if(itIn->hasValue)
					{
						SloBScanDistanceMap::InfoBScanDist info1;
						SloBScanDistanceMap::InfoBScanDist info2;
						info1.bscan = itIn->val1.bscanId;
						info2.bscan = itIn->val2.bscanId;

#if true
						recalcDistDataL2(x, y, info1, itIn->val1);
						recalcDistDataL2(x, y, info2, itIn->val2);
#else
						info1.ascan    = itIn->val1.ascanId;
						info2.ascan    = itIn->val2.ascanId;
						info1.distance = itIn->val1.distance;
						info2.distance = itIn->val2.distance;
#endif

						if(info2.distance < info1.distance)
							std::swap(info1, info2);

						itOut->bscan1 = info1;
						itOut->bscan2 = info2;
						itOut->init   = true;
					}
					++itOut;
					++itIn;
				}
			}
		}


	public:
		FillPreCalcData(SloBScanDistanceMap::PreCalcDataMatrix& matrix, const OctData::Series& series)
		: matrix(matrix)
		, series(series)
		, pixelMap(matrix.getSizeX(), matrix.getSizeY())
		, transformCoord(series)
		{
			creatL1DistanceMap();
			fillPreCalcData();
		}
	};
}


SloBScanDistanceMap::SloBScanDistanceMap()
{
}


SloBScanDistanceMap::~SloBScanDistanceMap()
{
	delete preCalcDataMatrix;
}


void SloBScanDistanceMap::createData(const OctData::Series* series)
{
	if(!series)
		return;

	const cv::Mat& sloImageMat = series->getSloImage().getImage();
	if(sloImageMat.empty())
		return;


	PreCalcDataMatrix* oldPreCalcDataMatrix = preCalcDataMatrix;

	preCalcDataMatrix = new PreCalcDataMatrix(static_cast<std::size_t>(sloImageMat.cols)
	                                        , static_cast<std::size_t>(sloImageMat.rows));

	if(oldPreCalcDataMatrix)
		delete oldPreCalcDataMatrix;

	FillPreCalcData fpcd(*preCalcDataMatrix, *series);
}

