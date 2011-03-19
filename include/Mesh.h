/*
 *  Mesh.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/10/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef Mesh_h__
#define Mesh_h__

#include "dojo_common_header.h"

#include "Buffer.h"
#include "Vector.h"
#include "Color.h"

namespace Dojo 
{
	class Mesh : public Buffer 
	{
	public:
				
		static const uint FIELDS_NUMBER = 12;	
		static const uint VERTEX_PAGE_SIZE = 256;
		static const uint INDEX_PAGE_SIZE = 256;
		
		static const uint VERTEX_FIELD_SIZES[];
		
		enum VertexField
		{
			VF_POSITION2D,
			VF_POSITION3D,
			VF_UV,
			VF_UV_1,
			VF_UV_2,
			VF_UV_3,
			VF_UV_4,
			VF_UV_5,
			VF_UV_6,
			VF_UV_7,
			VF_COLOR,
			VF_NORMAL,
		};
		
		enum TriangleMode
		{
			TM_STRIP,
			TM_LIST
		};
				
		Mesh( ResourceGroup* creator = NULL, const std::string& filePath = "" ) :
		Buffer( creator, filePath ),
		vertexSize(0),
		vertexCount( 0 ),		
		vertexMaxCount( 0 ),
		currentVertex( NULL ),
		vertices( NULL ),
		indexCount( 0 ),
		indexMaxCount( 0 ),
		indices( NULL ),
		dynamic( false ),
		editing( false ),
		triangleMode( TM_STRIP ),
		vertexHandle(0),
		indexHandle(0),
		currentUVSet(0)
		{
			//set all fields to zero
			memset( vertexFields, 0, sizeof( bool ) * FIELDS_NUMBER );
			memset( vertexFieldOffset, 0, sizeof(uint) * FIELDS_NUMBER );
		}		
		
		virtual ~Mesh()
		{
			//destroy CPU mem
			_destroyBuffers();
			
			//and GPU mem
			unload();
		}
		
		void setVertexFieldEnabled( VertexField f )
		{			
			if( !vertices ) //if the memory has not been created
			{				
				vertexFields[f] = true;
				
				vertexSize += VERTEX_FIELD_SIZES[ f ];
			}
		}
		
		///Tells the buffer to allocate at least "vertices" vertices
		inline void setVertexCap( uint count )
		{		
			if( count < vertexMaxCount ) //no need to grow the buffer
				return;
			
			vertexMaxCount = (count/VERTEX_PAGE_SIZE + 1 ) * VERTEX_PAGE_SIZE;
			
			if( !vertices )
			{					
				vertices = (byte*)malloc( vertexSize * vertexMaxCount );
				memset( vertices, 0, vertexMaxCount * vertexSize );
				
				_buildFieldOffsets();	//build the offsets for each field	
			}			
			else
			{	
				//TODO MAKE THIS ACTUALLY WORK
				vertices = (byte*)realloc( vertices, vertexSize * vertexMaxCount );
			}
		}	

		inline void setIndexCap( uint count )
		{
			if( count < indexMaxCount ) //no need to grow the buffer
				return;

			vertexMaxCount = (count/VERTEX_PAGE_SIZE + 1 ) * VERTEX_PAGE_SIZE;

			if( !vertices )
			{					
				indices = (GLint*)malloc( sizeof(GLint) * vertexMaxCount );
				memset( indices, 0, sizeof(GLint) * vertexMaxCount );
			}			
			else
			{	
				//TODO MAKE THIS ACTUALLY WORK
				indices = (GLint*)realloc( indices, sizeof(GLint) * vertexMaxCount );
			}
		}
		
		///A dynamic mesh won't clear its CPU cache when loaded, and allows to call load() more than one time!
		inline void setDynamic( bool d )
		{
			dynamic = d;
		}		
		
		inline void setTriangleMode( TriangleMode m )	{	triangleMode = m;	}
		
		inline TriangleMode getTriangleMode()	{	return triangleMode;	}

		inline const Vector& getMax()
		{
			return max;
		}

		inline const Vector& getMin()
		{
			return min;
		}

		inline const Vector& getDimensions()
		{
			return dimensions;
		}

		inline const Vector& getCenter()
		{
			return center;
		}
		
		///tells if begin() has been called not followed by an end()
		inline bool isEditing()
		{
			return editing;
		}
					
		inline void begin( uint extimatedVerts = 1 )
		{			
			//be sure that we aren't already building
			DEBUG_ASSERT( extimatedVerts > 0 );
			DEBUG_ASSERT( !isEditing() );

			//the buffer is too small for extimated vertex count?
			setVertexCap( extimatedVerts );
			
			currentVertex = NULL;
			vertexCount = 0;
			indexCount = 0;

			max.x = max.y = -FLT_MAX;
			min.x = min.y = FLT_MAX;
			
			editing = true;			
		}

		///starts the update after the last added vertex - useful for sequential updates
		inline void append()
		{
			DEBUG_ASSERT( !isEditing() );

			currentVertex = vertices + vertexSize * (vertexCount-1);

			editing = true;
		}
					
		///adds a vertex at the given position
		void vertex( float x, float y );
		
		///adds a vertex at the given position
		void vertex( float x, float y, float z );

		inline void vertex( const Vector& v )
		{
			vertex( v.x, v.y, v.z );
		}
				
		///sets the uv of the last set vertex				
		inline void uv( float u, float v )
		{			
			DEBUG_ASSERT( isEditing() );
			
			float* ptr = (float*)( currentVertex + vertexFieldOffset[ VF_UV + currentUVSet ] );
			*ptr++ = u;
			*ptr = v;

			++currentUVSet;
		}
		
		//sets the color of the last set vertex		
		inline void color( float r, float g, float b, float a  )
		{		
			DEBUG_ASSERT( isEditing() );	
			
			GLubyte* ptr = (GLubyte*)( currentVertex + vertexFieldOffset[ VF_COLOR ] );
			*ptr++ = (GLubyte)(r*255);
			*ptr++ = (GLubyte)(g*255);
			*ptr++ = (GLubyte)(b*255);
			*ptr = (GLubyte)(a*255);
		}

		inline void color( const Color& c )
		{
			color( c.r, c.g, c.b, c.a );
		}
		
		///adds a vertex at the given position
		inline void normal( float x, float y, float z )
		{		
			DEBUG_ASSERT( isEditing() );
			
			float* ptr = (float*)( currentVertex + vertexFieldOffset[ VF_NORMAL ] );
			
			*ptr++ = x;
			*ptr++ = y;
			*ptr = z;
		}
		
		inline void normal( const Vector& n )
		{
			normal( n.x, n.y, n.z );
		}

		///adds one index
		inline void index( uint idx )
		{		
			DEBUG_ASSERT( isEditing() );
			
			setIndexCap( indexCount + 1 );

			indices[ indexCount++ ] = idx;			
		}
		
		inline void triangle( uint i1, uint i2, uint i3 )
		{
			index(i1);
			index(i2);
			index(i3);
		}

		inline void quad(uint i11, uint i12, uint i21, uint i22 )
		{
			triangle(i11,i21,i12);
			triangle(i21,i22,i12);
		}

		///loads the data on the device
		/**
		-will discard all the data if the buffer is static
		-if the buffer is dynamic, this can be called again to update device data
		*/
		bool end();
		
		///loads the whole file passed in the constructor
		virtual bool load();
		
		
		virtual void unload()
		{
			if( loaded )
			{
				glDeleteBuffers( 1, &vertexHandle );
				glDeleteBuffers( 1, &indexHandle );


				glBindBuffer(GL_ARRAY_BUFFER, 0);
				
				vertexHandle = indexHandle = 0;
				
				loaded = false;
			}
		}
				
		///binds all the pointers for the needed client states
		virtual void bind();
				
		inline bool isIndexed()
		{
			return indices || indexHandle;
		}
		
		inline bool isVertexFieldEnabled( VertexField f )
		{
			return vertexFields[f];
		}
		
		inline uint getVertexCount()
		{
			return vertexCount;
		}
		
		inline uint getIndexCount()
		{
			return indexCount;
		}
		
		///obtains a copy of this mesh
		/**
		It has to be loaded by hand!
		*/
		inline Mesh* getCopy()
		{
			Mesh* copy = new Mesh( creator, filePath );
			
			copy->dynamic = dynamic;
			copy->vertexSize = vertexSize;			
			memcpy( copy->vertexFields, vertexFields, sizeof( bool ) * FIELDS_NUMBER );
			
			copy->vertexCount = vertexCount;						
			copy->indexCount = indexCount;			
			
			copy->setVertexCap( copy->vertexCount );
			copy->setIndexCap( copy->indexCount );
			
			memcpy( copy->vertices, vertices, vertexSize * vertexMaxCount );
			memcpy( copy->indices, indices, sizeof( uint ) * indexCount );
			
			return copy;
		}
		
		inline float* _getVertex( uint i )
		{			
			DEBUG_ASSERT( i < vertexCount );
			
			return (float*)(vertices + vertexSize * i);
		}
				
	protected:

		Vector max, min, center, dimensions;
				
		uint vertexSize, vertexCount, vertexMaxCount;
		byte* currentVertex;
		byte* vertices;
		
		uint indexCount, indexMaxCount;
		GLint* indices;		

		uint currentUVSet;
		
		bool vertexFields[ FIELDS_NUMBER ];		
		uint vertexFieldOffset[ FIELDS_NUMBER ];
		
		TriangleMode triangleMode;
		
		uint vertexHandle, indexHandle;
		
		bool dynamic;
		bool editing;
				
		void _buildFieldOffsets()
		{
			uint offset = 0;
			for( uint i = 0; i < FIELDS_NUMBER; ++i )
			{
				if( isVertexFieldEnabled( (VertexField)i ) )
				{
					vertexFieldOffset[ i ] = offset;
					offset += VERTEX_FIELD_SIZES[ i ];
				}
			}			
		}
				
		void _destroyBuffers()
		{
			if( vertices )
			{
				free( vertices );
				vertices = NULL;
			}
			
			if( indices )
			{
				free( indices );
				indices = NULL;
			}
		}

		void _prepareVertex( float x, float y, float z );
	};
}

#endif
