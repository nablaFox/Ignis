#version 450

#extension GL_EXT_buffer_reference : require

layout(location = 1) out vec3 outColor;

struct Vertex {
    vec3 position;
    vec4 color;
};

struct DirectionalLight {
    vec3 direction;     
    vec3 color;     
};

layout(set = 0, binding = 0) uniform SceneData {   
    mat4 viewproj;
    vec4 ambientColor;
    DirectionalLight sun;
} sceneData;

struct Color {
	vec4 color;
};

layout(set = 0, binding = 1) uniform TestData {   
	Color color;
} testData;

// declares a pointer to a vertex buffer
layout(buffer_reference, std430) readonly buffer VertexBuffer { 
    Vertex vertices[];
};

layout(push_constant) uniform constants {	
    mat4 worldTransform;
    VertexBuffer meshVertexBuffer;
} PushConstants;

void main() {	
    Vertex v = PushConstants.meshVertexBuffer.vertices[gl_VertexIndex];
    vec4 position = vec4(v.position, 1.0f);

    gl_Position = sceneData.viewproj * PushConstants.worldTransform * position;

    outColor = v.color.xyz;
}
