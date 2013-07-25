#ifndef Tessellation_h__
#define Tessellation_h__

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo
{
	
	///A Tessellation is a 2D triangle mesh created by the tessellation of an area enclosed by edges using Constrained Delaunay Triangulation
	/**
	The tessellation class can tessellate any given countour mesh that is initialized in its "positions" and "indices" arrays.

	\remark if a contour is not closed or one or more contours intersect, the results of the tessellation are undefined
	*/
	class Tessellation
	{
	public:

		struct Position
		{
			double x, y;

			Position( const Vector& p ) :
				x( p.x ),
				y( p.y )
			{

			}

			Position( double X, double Y ) : 
				x( X ),
				y( Y )
			{

			}

			Vector toVec()
			{
				return Vector( (float)x, (float)y );
			}
		};

		struct Segment
		{
			int i1, i2;

			Segment( int a, int b ) :
				i1( a ),
				i2( b )
			{
				DEBUG_ASSERT( a >= 0, "Invalid negative index" );
				DEBUG_ASSERT( b >= 0, "Invalid negative index" );
				DEBUG_ASSERT( a != b, "A segment can't start and end at the same vertex" );
			}

			int& operator[] ( int i )
			{
				return i == 0 ? i1 : i2;
			}
		};

		///a Loop defines a closed circuit of segments using their start and end index-indices
		struct Contour
		{
			std::vector< int > indices;

			int parity;
			bool closed;

			Contour( ) :
				parity( -1 ),
				closed( false )
			{

			}

			///returns the nth segment of the contour - it is unbounded, so oob locations are wrapped into the contour
			int operator[]( int n )
			{
				while( n < 0 )
					n += indices.size();

				return indices[ n % indices.size() ];
			}

			///adds a segment to this contour, marks it as closed if end == start
			void _addSegment( int start, int end )
			{
				indices.push_back( start );
				indices.push_back( end );
				
				closed = (end == indices.front());
			}
		};

		typedef std::vector< Contour > ContourList;
		typedef std::vector< Segment > SegmentList;

		//in
		std::vector< Position > positions;
		SegmentList segments;

		//mid
		ContourList contours;
		std::vector< int > contourForSegment;
		std::vector< Position > holes;

		struct ExtrusionVertex
		{
			Vector position, normal;

			ExtrusionVertex( const Position& p ) :
				position( (float)p.x, (float)p.y )
			{

			}

			ExtrusionVertex( const Vector& p, const Vector& n ) :
				position( p ),
				normal( n )
			{

			}
		};

		std::vector< ExtrusionVertex > extrusionContourVertices;
		SegmentList extrusionContourIndices;

		//out
		std::vector< int > outIndices;

		///Creates an empty 2D Tesselation object
		Tessellation()
		{

		}

		///Adds a 2D point to the tessellation contour
		void addPoint( const Vector& p )
		{
			positions.push_back( p );
		}

		///adds a point and the indices to construct a single segment starting from the last point
		void addSegment( const Vector& p )
		{
			int idx = positions.size()-1;
			addPoint( p );

			//add indices to the point
			segments.push_back( Segment( idx, idx+1 ) );
		}

		///adds a quadratic bezier curve (single control point) starting from the last point
		void addQuadradratic( const Vector& B, const Vector& C, float pointsPerUnitLength )
		{
			Vector U, V, A = positions.back().toVec();

			//TODO actually add points evaluating the "curvyness" of the path
			float length = A.distance( B ) + B.distance( C ); //compute a rough length of this arc
			int subdivs = (int)(length * pointsPerUnitLength + 1);

			for(int i = 1; i <= subdivs; i++)
			{
				float t = (float)i / subdivs;
				
				U = A.lerpTo( t, B );
				V = B.lerpTo( t, C );

				addSegment( U.lerpTo( t, V ) );
			}
		}

		///adds a cubic bezier curve (double control point) starting from the last point
		void addCubic( const Vector& B, const Vector& C, const Vector& D, float pointsPerUnitLength )
		{
			Vector U,V,W,M,N, A = positions.back().toVec();
			
			//TODO actually add points evaluating the "curvyness" of the path
			float length = A.distance( B ) + B.distance( C ) + C.distance( D ); //compute a rough length of this arc
			int subdivs = (int)(length * pointsPerUnitLength + 1);

			for( int i = 0; i <= subdivs; i++)
			{
				float t = (float)i / subdivs;

				U = A.lerpTo( t, B );
				V = B.lerpTo( t, C );
				W = C.lerpTo( t, D );

				M = U.lerpTo( t, V );
				N = V.lerpTo( t, W );

				addSegment( M.lerpTo( t, N ) );
			}
		}
		
		///removes i2 from the point list and rearranges all the indices to point to i1
		void mergePoints( int i1, int i2 );

		///merges all the points that share the same position
		/**
		this method will be automatically run by tessellate() as the triangulation algorithm doesn't allow for duplicate points
		*/
		void mergeDuplicatePoints();

		///generates an extrusion contour mesh - it is different from the normal contour because vertices with an excessive angles are split
		void generateExtrusionContour();

		///builds the internal "loops" structure, representing all the contours of this tessellation
		/**
		each loop contains a copy of all of its segments
		*/
		void findContours();

		///tessellates the countour mesh producing a triangle mesh
		/**
		\param clearInputs auto-clears the input vectors
		*/
		void tessellate( bool clearInputs = true );

	protected:

		bool _raycastSegmentAlongX( const Segment& segment, const Position& startPosition );

		int _assignToIncompleteContour( int start, int end );

		void _assignNormal( const Vector& n, Segment& s, int i, SegmentList& additionalSegmentsBuffer );
	};
}

#endif // Tesselation_h__