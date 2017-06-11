import struct

class vertex:
	def __init__(self,x,y,z):
		self.x = float(x)
		self.y = float(y)
		self.z = float(z)

	def write(self,file):
		file.write(struct.pack("<3f",self.x,self.y,self.z))

class uv:
	def __init__(self,x,y):
		self.u = float(x)
		self.v = float(y)

	def write(self,file):
		a = int( ((self.u+1)*0.5) * 65535 )
		b = int( ((self.v+1)*0.5) * 65535 )
		file.write(struct.pack("<2H",a,b))

class normal:
	def __init__(self,x,y,z):
		self.x = float(x)
		self.y = float(y)
		self.z = float(z)

	def write(self,file):
		file.write(struct.pack("<3f",self.x,self.y,self.z))

class packed_vertex:
	def __init__(self,v,uv,n):
		self.vertex = v
		self.normal = n
		self.uv = uv

	def write(self,file):
#		print(self.vertex)
		self.vertex.write(file)
		if (self.normal != None):
			self.normal.write(file)
		if (self.uv != None):
			self.uv.write(file)


vertices = []
uv_coord = []
normals = []

packed_vertices = []
face_map = {}

indices = []

def push_index(token):
	if token in face_map:
		indices.append( face_map[token] )
	else:
		idx = len(packed_vertices);
		face_map[token] = idx
		indices.append(idx)
		token = token.replace("//","/")
		i = list(map(lambda x: int(x)-1,token.split("/")));
		pv = ''

		if (len(i) == 3):
			pv = packed_vertex(vertices[i[0]],uv_coord[i[1]],normals[i[2]])
		elif (len(i) == 2):
			print(i[0], i[0])
			pv = packed_vertex(vertices[i[0]],uv(0,0),normals[i[1]])
		elif (len(i) == 1):
			pv = packed_vertex(vertices[i[0]],None,None)
		else:
			print("Unknown face format")
			exit(1)

		packed_vertices.append(pv)

f = open('test.obj', 'r')

for line in f:
	tokens = line.split();
	if (tokens[0] == "v"):
		vertices.append( vertex(tokens[1],tokens[2],tokens[3]) )

	if (tokens[0] == "vt"):
		uv_coord.append( uv(tokens[1],tokens[2]) )

	if (tokens[0] == "vn"):
		normals.append( normal(tokens[1],tokens[2],tokens[3]) )

	if (tokens[0] == "f"):
		push_index(tokens[1])
		push_index(tokens[2])
		push_index(tokens[3])

output = open('test.hgmdl', 'wb')
output.write(struct.pack("<2I",len(packed_vertices),len(indices)))
for pv in packed_vertices:
	print(pv.vertex.x,pv.vertex.y,pv.vertex.z)
	pv.write(output)

output.write(struct.pack('<'+'H'*len(indices),*indices))

print ('vertices:', len(packed_vertices))
print ('indices:', len(indices))