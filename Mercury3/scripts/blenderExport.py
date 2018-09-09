import bpy
import struct

vertices = [];
uvs = [];

class MeshData:
    def __init__(self):
        self.vertexOffset = 0; #offset into vertex buffer
        self.indices = [];
        # self.uvs = [];


bl_info = {"name": "HgMDL Model Exporter", "category": " Import-Export"}
# Only needed if you want to add into a dynamic menu
def menu_func_export(self, context):
    self.layout.operator(ExportToHgMDL.bl_idname, text=bl_info['name'])


def register():
    bpy.utils.register_class(ExportToHgMDL)
    bpy.types.INFO_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportToHgMDL)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)

def getMeshData(o):
    global vertices;
    global uvs;

    mesh = o.data;
    md = MeshData();
    md.offset = len(vertices);
    vertices = vertices + mesh.vertices.values();
    for face in mesh.polygons:
        indices = face.vertices;
        if len(indices) != 3:
            raise ValueError('Faces must be triangles')
        for x in indices:
            md.indices.append(x+md.offset);

    uv_layer = mesh.uv_layers[0]
    uvs = [i.uv for i in uv_layer.data]

    return md;

def UVtoInt16(u,v):
    u = float(u)
    v = float(v)
    a = int(u * 65535);
    b = int(v * 65535);
    a = max(0, min(a,0xffff))
    b = max(0, min(b,0xffff))
    return (a,b);


class VertexUVCombiner:
    def __init__(self,vIdx,u,v):
        self.vertex_index = vIdx
        self.u = u
        self.v = v

    def __hash__(self):
        return hash((self.vertex_index, self.u, self.v))

    def __eq__(self, o):
        return (self.vertex_index, self.u, self.v) == (o.vertex_index, o.u, o.v)

def expandVertices(indices):
    #there is one UV for each vertex index, so UVs share vertices
    #we can not render this so vertices need to be duplicated and assigned
    #their unique UV coordinates
    expandedVertices = []
    newIndices = [] #index mapping into new expandedVertices list
    vertexDict = dict() #for deduplication quick lookup

    for i in range(len(indices)):
        uv = uvs[i]
        vIdx = indices[i]
        key = VertexUVCombiner(vIdx, uv.x, uv.y)
        if key in vertexDict:
            idx = vertexDict[key]
            newIndices.append(idx)
        else:
            idx = len(expandedVertices)
            expandedVertices.append(key)
            vertexDict[key] = idx
            newIndices.append(idx)

    return (expandedVertices,newIndices)

def write_some_data(context, filepath, use_some_setting):
    global vertices;
    selectedObjects = bpy.context.selected_objects;
    meshData = 0;
    for o in selectedObjects:
        if o.type == 'MESH':
            meshData = getMeshData(o);
            break;

    (expandedVerts, remapedIndices) = expandVertices(meshData.indices)

    print("vertex count ", len(vertices))
    print("uv count ", len(uvs))
    print("indices count ", len(remapedIndices))
    print("expanded vertex count ", len(expandedVerts))

    output = open( filepath, 'wb')
    output.write(struct.pack("<2I",len(expandedVerts),len(remapedIndices)))

    for ev in expandedVerts:
        vertex = vertices[ev.vertex_index]
        co = vertex.co
        n = vertex.normal
        (uv_x,uv_y) = UVtoInt16(ev.u,ev.v);
        output.write(struct.pack("<3f", co.x, co.y, co.z))
        output.write(struct.pack("<3f", n.x, n.y, n.z))
        output.write(struct.pack("<4f",0,0,0,0)) #tangent
        output.write(struct.pack("<2H", uv_x, uv_y))

    output.write(struct.pack('<'+'H'*len(remapedIndices),*remapedIndices))
    output.close()

    return {'FINISHED'}


# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator


class ExportToHgMDL(Operator, ExportHelper):
    """Export meshes to HgMDL format"""
    bl_idname = "export_hgmdl.data"  # important since its how bpy.ops.export_hgmdl.data is constructed
    bl_label = "Export"

    # ExportHelper mixin class uses this
    filename_ext = ".hgmdl"

    filter_glob = StringProperty(
            default="*.txt",
            options={'HIDDEN'},
            maxlen=255,  # Max internal buffer length, longer would be clamped.
            )

    # List of operator properties, the attributes will be assigned
    # to the class instance from the operator settings before calling.
    use_setting = BoolProperty(
            name="Example Boolean",
            description="Example Tooltip",
            default=True,
            )

    type = EnumProperty(
            name="Example Enum",
            description="Choose between two items",
            items=(('OPT_A', "First Option", "Description one"),
                   ('OPT_B', "Second Option", "Description two")),
            default='OPT_A',
            )

    def execute(self, context):
        return write_some_data(context, self.filepath, self.use_setting)

if __name__ == "__main__":
    register()

    # test call
    bpy.ops.export_hgmdl.data('INVOKE_DEFAULT')
