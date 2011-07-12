import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;


public class Mesh {
	
	class Vector {
		public float x, y, z;
		
		public Vector()
		{
			
		}
		
		public Vector( float x, float y, float z ) {
			this.x = x;
			this.y = y;
			this.z = z;
		}
	}
		
	class UV {
		public float u,v;
	}
	
	class Vertex {
		public Vector p, n;
		public UV uv;
		
		public int index;
		
		public boolean equals( Object o ) {
			if( o.getClass() != this.getClass() )
				return false;
			
			Vertex v = (Vertex)o;
			
			return v.p == p && v.uv == uv && v.n == n;
		}
	}
	
	class Face {
		public int i1, i2, i3;
		
		public Face( int a, int b, int c ) {
			i1 = a;
			i2 = b;
			i3 = c;
		}
	}
	
	List< Vector > pos = new ArrayList< Vector >();
	List< UV> uv = new ArrayList< UV >();
	List< Vector > normal = new ArrayList< Vector >();
	
	List< Vertex > vertices = new ArrayList< Vertex >();
	
	List< Face > face = new ArrayList< Face >();
	
	Vector max = new Vector( -Float.MAX_VALUE, -Float.MAX_VALUE, -Float.MAX_VALUE );
	Vector min = new Vector( Float.MAX_VALUE, Float.MAX_VALUE, Float.MAX_VALUE );
	
	public Mesh( String infile ) throws IOException {
		parse( infile );
	}
	
	public int getVertexCount() {
		return vertices.size();
	}
	
	public int getIndexCount() {
		return face.size()*3;
	}
	
	public int addOrRetrieveIndex( Vertex v ) {
		
		int idx = vertices.indexOf( v );
		
		if( idx == -1 ) { //new, add to the list
			idx = vertices.size();
			
			vertices.add( v );
			v.index = idx;
		}
		
		return idx;
	}
	
	public int parseVertex( StringTokenizer tk ) {
		Vertex v = new Vertex();
		
		//retrieve values
		v.p = pos.get( Integer.parseInt( tk.nextToken("/ ") )-1 );
		v.uv = uv.get( Integer.parseInt( tk.nextToken("/ ") )-1 );
		v.n = normal.get( Integer.parseInt( tk.nextToken("/ ") )-1 );
		
		//max and min
		max.x = Math.max( v.p.x, max.x );
		max.y = Math.max( v.p.y, max.y );
		max.z = Math.max( v.p.z, max.z );
		
		min.x = Math.min( v.p.x, min.x );
		min.y = Math.min( v.p.y, min.y );
		min.z = Math.min( v.p.z, min.z );
								
		//find its index
		return addOrRetrieveIndex( v );
	}
	
	public void parse( BufferedReader in ) throws IOException {
		
		
		while( in.ready() ) {
			StringTokenizer tk = new StringTokenizer( in.readLine() );
			
			while( tk.hasMoreTokens() ) {
				
				String s = tk.nextToken();
				
				if( s.equals( "v" ) ) {
					Vector v = new Vector();
					
					v.x = Float.parseFloat( tk.nextToken() );
					v.y = Float.parseFloat( tk.nextToken() );
					v.z = Float.parseFloat( tk.nextToken() );
					
					pos.add( v );
				}
				else if( s.equals( "vt" ) ) {
					UV v = new UV();
					
					v.u = Float.parseFloat( tk.nextToken() );
					v.v = 1.f - Float.parseFloat( tk.nextToken() );
										
					uv.add( v );					
				}
				else if( s.equals( "vn" ) ) {
					Vector v = new Vector();
					
					v.x = Float.parseFloat( tk.nextToken() );
					v.y = Float.parseFloat( tk.nextToken() );
					v.z = Float.parseFloat( tk.nextToken() );
					
					normal.add( v );
				}
				else if( s.equals( "f" ) ) {
					
					int idx[] = new int[4];
					
					//read faces
					for( int i = 0; i < 3; ++i )
						idx[i] = parseVertex( tk );

					face.add( new Face( idx[0], idx[1], idx[2] ) );
					
					//is it a quad?
					if( tk.hasMoreTokens() ) {
						idx[3] = parseVertex( tk );
						
						face.add( new Face( idx[0], idx[2], idx[3] ) );		
					}
				}
				else break;
			}
		}
	}
	
	public void parse( String infile ) throws IOException {
		
		parse( new BufferedReader( new FileReader( infile ) ) );
	}
	
	public void write( EndiannessFilterStream out ) throws IOException {
		
		/*format is
		| byte index size | byte triangle mode | byte field 1 | ... | byte field N | Vector max | Vector min
		| vertex data | index data |
		
		where the fields are
		enum VertexField
		{
			VF_POSITION2D = 0,
			VF_POSITION3D = 1,
			VF_UV = 2,
			VF_UV_1 = 3,
			VF_UV_2 = 4,
			VF_UV_3 = 5,
			VF_UV_4 = 6,
			VF_UV_5 = 7,
			VF_UV_6 = 8,
			VF_UV_7 = 9,
			VF_COLOR = 10,
			VF_NORMAL = 11
		};*/
		
		//either 8 bit or 16 bit indices
		int indexBytes = (getIndexCount()) < 0xff ? 1 : 2; 
		out.write( indexBytes );
		
		//always triangle list
		out.write( 1 );
		
		//set flags - by default only Pos3D, UV and NORMAL are enabled
		out.write(0);  //2D
		out.write(1);  //3D
		
		out.write(1); //UVS
		out.write(0);
		out.write(0);
		out.write(0);
		out.write(0);
		out.write(0);
		out.write(0);
		out.write(0);
		
		out.write(0); //color
		
		out.write(1); //normal
		
		//max
		out.writeFloat( max.x );
		out.writeFloat( max.y );
		out.writeFloat( max.z );

		//min
		out.writeFloat( min.x );
		out.writeFloat( min.y );
		out.writeFloat( min.z );
				
		//write vertex number
		out.writeInt( getVertexCount() );
		
		//write index number
		out.writeInt( getIndexCount() );
				
		//write down interleaved vertex data
		for( int i = 0; i < vertices.size(); ++i ) {
			Vertex v = vertices.get(i);
			
			out.writeFloat( v.p.x );
			out.writeFloat( v.p.y );
			out.writeFloat( v.p.z );
			
			out.writeFloat( v.uv.u );
			out.writeFloat( v.uv.v );
			
			out.writeFloat( v.n.x );
			out.writeFloat( v.n.y );
			out.writeFloat( v.n.z );
		}
		
		//write down index data
		if( indexBytes == 1 ) {
			for( Face f : face ) {
				out.write( f.i1 );
				out.write( f.i2 );
				out.write( f.i3 );
			}
		}
		else if( indexBytes == 2 ) {
			for( Face f : face ) {
				out.writeShort( (short)f.i1 );
				out.writeShort( (short)f.i2 );
				out.writeShort( (short)f.i3 );
			}
		}
	}
	
	public void write( String outfile, ByteOrder endianness) throws IOException {
		
		//open file
		write(	new EndiannessFilterStream(	endianness,	new FileOutputStream( outfile ) ) );
	}
}
