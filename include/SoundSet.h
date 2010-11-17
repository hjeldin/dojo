#ifndef SoundData_h__
#define SoundData_h__

#include "dojo_config.h"

#include "Array.h"
#include "SoundBuffer.h"

#include "Math.h"

namespace Dojo
{
	
	class SoundSet
	{
	public:

		SoundSet( const std::string& setName ) :
		name( setName ),
		buffers( 1,1 )	//pagina minima, il vettore e' statico
		{
			
		}

		///restituisce un buffer casuale
		inline SoundBuffer* getBuffer()
		{
			if( buffers.size() == 0 )
				return NULL;
			else if( buffers.size() == 1 )
				return buffers[0];
			else
			{
				float i = Math::rangeRandom( 0, buffers.size() );
				return buffers.at( (int)i ); 
			}
		}

		inline SoundBuffer* getBuffer( int i )
		{
			DOJO_ASSERT( buffers.size() > i );
			
			return buffers.at(i);
		}

		inline int getBufferNb()	{	return buffers.size();	 }

		inline const std::string& getName()	{	return name;	}

		inline void addBuffer( SoundBuffer* b )
		{
			DOJO_ASSERT( b );
			
			buffers.addElement( b );
		}

	protected:

		std::string name;

		Array<SoundBuffer*> buffers;
	};
}

#endif
