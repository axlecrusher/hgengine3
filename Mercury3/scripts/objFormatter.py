import struct
import sys
import math

class vertex:
	def __init__(self,x,y,z):
		self.x = float(x)
		self.y = float(y)
		self.z = float(z)

	def __add__(self, other):
		x = self.x + other.x
		y = self.y + other.y
		z = self.z + other.z
		return vertex(x,y,z)

	def __sub__(self, other):
		x = self.x - other.x
		y = self.y - other.y
		z = self.z - other.z
		return vertex(x,y,z)

	def normalize(self):
		length = math.sqrt((self.x * self.x) + (self.y * self.y) + (self.z * self.z))
		return vertex(self.x/length, self.y/length,self.z/length)

	def write(self,file):
		file.write(struct.pack("<3f",self.x,self.y,self.z))
	def write4(self,file):
		file.write(struct.pack("<4f",self.x,self.y,self.z,0))
#		file.write(struct.pack("<4f",self.x,self.y,self.z,self.w))
	def hex(self,file):
		data = struct.pack("<3f",self.x,self.y,self.z)
		file.write(",".join('0x%X'%x for x in struct.iter_unpack("I",data))+ ',')
	def text(self,file):
		file.write(', '.join((str(self.x),str(self.y),str(self.z))) + ', ' )

def dot(self, other):
	r = (self.x*other.x)
	+ (self.y*other.y)
	+ (self.z*other.z)
	return r

def cross(a,b):
	r = vertex(0,0,0)
	r.x = (a.y * b.z) - (a.z * b.y)
	r.y = (a.z * b.x) - (a.x * b.z)
	r.z = (a.x * b.y) - (a.y * b.x)
	return r

class uv:
	def __init__(self,x,y):
		self.u = float(x)
		self.v = float(y)
#		clamp();

	def toInt16(self):
#		a = int( ((self.u+1)*0.5) * 65535 )
#		b = int( ((self.v+1)*0.5) * 65535 )
		a = int(self.u * 65535);
		b = int(self.v * 65535);
		a = max(0, min(a,0xffff))
		b = max(0, min(b,0xffff))
		return (a,b);

	def write(self,file):
		(a,b) = self.toInt16()
		file.write(struct.pack("<2H",a,b))
	def hex(self,file):
		(a,b) = self.toInt16()
		data = struct.pack("<2H",a,b)
		file.write(",".join('0x%X'%x for x in struct.iter_unpack("I",data))+ ',')
	def text(self,file):
		(a,b) = self.toInt16()
		file.write(', '.join((str(a),str(b))) + ', ' )


class normal:
	def __init__(self,x,y,z):
		self.x = float(x)
		self.y = float(y)
		self.z = float(z)

	def __mul__(self,f):
		r = normal(self.x,self.y,self.z)
		r.x *= f
		r.y *= f
		r.z *= f
		return r

	def write(self,file):
		file.write(struct.pack("<3f",self.x,self.y,self.z))
	def hex(self,file):
		data=struct.pack("<3f",self.x,self.y,self.z)
		file.write(",".join('0x%X'%x for x in struct.iter_unpack("I",data))+ ',')
	def text(self,file):
		file.write(', '.join((str(self.x),str(self.y),str(self.z))) + ', ' )

class packed_vertex:
	def __init__(self,v,uv,n):
		self.vertex = v
		self.normal = n
		self.uv = uv
		self.tangent = vertex(0,0,0) #tangent needs to be its own data type

	def write(self,file):
#		print(self.vertex)
		self.vertex.write(file)
		if (self.normal != None):
			self.normal.write(file)
			self.tangent.write4(file)
		if (self.uv != None):
			self.uv.write(file)

	def hex(self,file):
#		print(self.vertex)
		self.vertex.hex(file)
		if (self.normal != None):
			self.normal.hex(file)
			self.tangent.hex(file)
		if (self.uv != None):
			self.uv.hex(file)

	def text(self,file):
#		print(self.vertex)
		self.vertex.text(file)
		if (self.normal != None):
			self.normal.text(file)
			self.tangent.text(file)
		if (self.uv != None):
			self.uv.text(file)

class triangle:
	def __init__(self,i1,i2,i3):
		self.index = [int(i1),int(i2),int(i3)]

#raw data. Index into them for faces
vertices = []
uv_coord = []
normals = []


packed_vertices = []
triangles = [] #built from indexing into packed_vertices
face_map = {}

indices = []
tangent = []

def CalculateTangentArray():
#    Vector3D *tan1 = new Vector3D[vertexCount * 2];
#    Vector3D *tan2 = tan1 + vertexCount;
#    ZeroMemory(tan1, vertexCount * sizeof(Vector3D) * 2);
	tan1 = []
	tan2 = []
	for v in packed_vertices:
		tan1.append(vertex(0,0,0))
		tan2.append(vertex(0,0,0))

	for triangle in triangles:
		i1 = triangle.index[0];
		i2 = triangle.index[1];
		i3 = triangle.index[2];
		
		v1 = packed_vertices[i1].vertex;
		v2 = packed_vertices[i2].vertex;
		v3 = packed_vertices[i3].vertex;
		
		w1 = packed_vertices[i1].uv;
		w2 = packed_vertices[i2].uv;
		w3 = packed_vertices[i3].uv;
		
		x1 = v2.x - v1.x;
		x2 = v3.x - v1.x;
		y1 = v2.y - v1.y;
		y2 = v3.y - v1.y;
		z1 = v2.z - v1.z;
		z2 = v3.z - v1.z;
		
		s1 = w2.u - w1.u;
		s2 = w3.u - w1.u;
		t1 = w2.v - w1.v;
		t2 = w3.v - w1.v;
		
		d = (s1 * t2 - s2 * t1);
		r = 1
		if (d!=0):
			r = 1.0 / d;
		sdir = vertex((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		tdir = vertex((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
		
		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;
		
		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;

	for a in range(0,len(packed_vertices)):
		n = packed_vertices[a].normal;
		t = tan1[a];

		# Gram-Schmidt orthogonalize
		packed_vertices[a].tangent = (t - n * dot(n, t)).normalize();
#		print (packed_vertices[a].tangent.x, packed_vertices[a].tangent.y, packed_vertices[a].tangent.z)
		# Calculate handedness
		packed_vertices[a].tangent.w = 1.0;
		if (dot(cross(n, t), tan2[a]) < 0.0):
			packed_vertices[a].tangent.w = -1.0;

def push_index(token):
	token = token.replace("//","/")
	if token in face_map:
		indices.append( face_map[token] )
	else:
		idx = len(packed_vertices);
		face_map[token] = idx
		indices.append(idx)
		i = list(map(lambda x: int(x)-1,token.split("/")));
		pv = ''

		length = len(i)
		if (length == 3):
			pv = packed_vertex(vertices[i[0]],uv_coord[i[1]],normals[i[2]])
		elif (length == 2):
#			print(i[0], i[0])
			pv = packed_vertex(vertices[i[0]],uv(0,0),normals[i[1]])
		elif (length == 1):
			pv = packed_vertex(vertices[i[0]],None,None)
		else:
			print("Unknown face format")
			exit(1)

		packed_vertices.append(pv)
	return face_map[token]

f = open( sys.argv[1], 'r')
currenLineNumber = 1
for line in f:
	print("Processing Line {}".format(currenLineNumber))
	currenLineNumber+=1
	tokens = line.split();
	if (tokens[0] == "v"):
		vertices.append( vertex(tokens[1],tokens[2],tokens[3]) )

	if (tokens[0] == "vt"):
		uv_coord.append( uv(tokens[1],tokens[2]) )

	if (tokens[0] == "vn"):
		normals.append( normal(tokens[1],tokens[2],tokens[3]) )

	if (tokens[0] == "f"):
		if (len(tokens)!=4): #includes "f" token
			print("Error: All faces must be triangles. ", len(tokens))
			exit(1)
		i1 = push_index(tokens[1])
		i2 = push_index(tokens[2])
		i3 = push_index(tokens[3])
		print(i1,i2,i3)
		triangles.append( triangle(i1,i2,i3) )

def output_binary():
	output = open( sys.argv[1]+'.hgmdl', 'wb')
	output.write(struct.pack("<2I",len(packed_vertices),len(indices)))
	for pv in packed_vertices:
	#	print(pv.vertex.x,pv.vertex.y,pv.vertex.z)
		pv.write(output);

	output.write(struct.pack('<'+'H'*len(indices),*indices))
	output.close()

def output_hex():
	output = open( sys.argv[1]+'.hex', 'w')
	for i in indices:
		packed_vertices[i].hex(output);
	output.close()

def output_text():
	output = open( sys.argv[1]+'.txt', 'w')
	for pv in packed_vertices:
		pv.text(output);
		output.write("\n")

	output.write("\n")
	for pv in packed_vertices:
		pv.hex(output);

	output.write("\n")
	output.write(', '.join(str(x) for x in indices) + ', ' )
#	output.text(struct.pack('<'+'H'*len(indices),*indices))
	output.close()

#CalculateTangentArray()

output_binary()
output_hex()
output_text()

print ('vertices:', len(packed_vertices))
print ('indices:', len(indices))