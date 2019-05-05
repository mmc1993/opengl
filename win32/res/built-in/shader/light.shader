Pass
	CullFace Back
	DepthTest
	DepthWrite
	RenderType Light
	DrawType Index

	Vertex
		#version 330 core

		layout(location = 0) in vec3 a_pos_;

		uniform mat4 matrix_mvp_;

        uniform int light_type_;

        #define LIGHT_TYPE_DIRECT_  0
        #define LIGHT_TYPE_POINT_   1
        #define LIGHT_TYPE_SPOT_    2

		void main()
		{
            switch (light_type_)
            {
            case LIGHT_TYPE_DIRECT_:
                gl_Position = vec4(a_pos_, 1);
                break;
            case LIGHT_TYPE_POINT_:
                gl_Position = matrix_mvp_ * vec4(a_pos_, 1);
                break;
            case LIGHT_TYPE_SPOT_:
                gl_Position = matrix_mvp_ * vec4(a_pos_, 1);
                break;
            }
		}
	End Vertex

	Fragment
		#version 330 core

        layout (std140) uniform LightDirect_ {
			mat4 mMatrix;
			vec3 mNormal;
			vec3 mAmbient;
			vec3 mDiffuse;
			vec3 mSpecular;
			vec3 mPosition;
        } light_direct_;

        layout (std140) uniform LightPoint_ {
			float mFar, mNear;
            float mK0, mK1, mK2;
            vec3 mAmbient;
            vec3 mDiffuse;
            vec3 mSpecular;
            vec3 mPosition;
        } light_point_;

        layout (std140) uniform LightSpot_ {
			float mK0;
            float mK1;
            float mK2;
			float mInCone;
            float mOutCone;
			mat4 mMatrix;
			vec3 mNormal;
			vec3 mAmbient;
			vec3 mDiffuse;
			vec3 mSpecular;
			vec3 mPosition;
        } light_spot_;

		uniform	sampler2D gbuffer_position_;
		uniform	sampler2D gbuffer_specular_;
		uniform	sampler2D gbuffer_diffuse_;
		uniform	sampler2D gbuffer_normal_;

		uniform vec3 camera_pos_;

        uniform int light_type_;

		out vec4 color_;

		void main()
		{
            gl_FragColor = vec4(1, 1, 1, 1);
		}
	End Fragment
End Pass