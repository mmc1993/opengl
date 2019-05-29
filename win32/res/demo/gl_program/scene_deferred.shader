Pass
	CullFace Front
	DepthTest
	DepthWrite
	RenderType Shadow
	DrawType Index

	Vertex
		#version 330 core

		layout(location = 0) in vec3 a_pos_;

		uniform mat4 matrix_mvp_;
		uniform mat4 matrix_m_;

		out V_OUT_ {
			vec3 mMPos;
		} v_out_;

		void main()
		{
			vec4 apos       = vec4(a_pos_, 1);
			v_out_.mMPos    = vec3(matrix_m_ * apos);
			gl_Position     = vec4(matrix_mvp_ * apos);
		}
	End Vertex

	Fragment
		#version 330 core

        layout (std140) uniform LightPoint_ {
			float mFar, mNear;
            float mK0, mK1, mK2;
            vec3 mAmbient;
            vec3 mDiffuse;
            vec3 mSpecular;
            vec3 mPosition;
        } light_point_;

		in V_OUT_{
			vec3 mMPos;
		} v_out_;

        uniform int light_type_;

        #define LIGHT_TYPE_DIRECT_ 0
        #define LIGHT_TYPE_POINT_ 1
        #define LIGHT_TYPE_SPOT_ 2

		void main()
		{
            switch (light_type_)
            {
            case LIGHT_TYPE_DIRECT_:
                {
                    gl_FragDepth = gl_FragCoord.z;
                }
                break;
            case LIGHT_TYPE_POINT_:
                {
					vec3 normal = v_out_.mMPos - light_point_.mPosition;
					gl_FragDepth = length(normal) / light_point_.mFar;
                }
                break;
            case LIGHT_TYPE_SPOT_:
                {
                    gl_FragDepth = gl_FragCoord.z;
                }
                break;
            }
		}
	End Fragment
End Pass

Pass
	CullFace Back
	DepthTest
	DepthWrite
	RenderQueue Geometric
	RenderType Deferred
	DrawType Index

	Vertex
		#version 330 core

		layout(location = 0) in vec3 a_pos_;
		layout(location = 1) in vec2 a_uv_;
		layout(location = 2) in vec3 a_n_;
		layout(location = 3) in vec3 a_t_;
		layout(location = 4) in vec3 a_b_;

		uniform vec3 camera_pos_;
		uniform vec3 camera_eye_;
		uniform mat4 matrix_mvp_;
		uniform mat4 matrix_mv_;
		uniform mat4 matrix_m_;
		uniform mat4 matrix_p_;
		uniform mat4 matrix_v_;
		uniform mat3 matrix_n_;

		out V_OUT_ {
			vec3 mNormal;
			vec4 mMVPPos;
			vec3 mMVPos;
			vec3 mMPos;
			mat3 mTBN;
			vec2 mUV;
		} v_out_;

		void main()
		{
			vec4 apos = vec4(a_pos_, 1);
			v_out_.mMPos = vec3(matrix_m_ * apos);
			v_out_.mMVPos = vec3(matrix_mv_ * apos);
			v_out_.mNormal = vec3(matrix_n_ * a_n_);
			v_out_.mMVPPos = vec4(matrix_mvp_ * apos);

			vec3 T = normalize(vec3(matrix_m_ * vec4(a_t_, 0.0f)));
			vec3 B = normalize(vec3(matrix_m_ * vec4(a_b_, 0.0f)));
			vec3 N = normalize(vec3(matrix_m_ * vec4(a_n_, 0.0f)));
			v_out_.mTBN = mat3(T, B, N);

			v_out_.mUV = a_uv_;

			gl_Position = v_out_.mMVPPos;
		}
	End Vertex

	Fragment
		#version 330 core

		uniform vec3 camera_pos_;
		uniform vec3 camera_eye_;

		uniform struct Material_ {
			float mShininess;
			sampler2D mNormal;
			sampler2D mSpecular;
			sampler2D mDiffuse0;
		} material_;

		in V_OUT_{
			vec3 mNormal;
			vec4 mMVPPos;
			vec3 mMVPos;
			vec3 mMPos;
			mat3 mTBN;
			vec2 mUV;
		} v_out_;

        layout (location = 0) out vec3 outPosition_;
        layout (location = 1) out vec4 outSpecular_;
        layout (location = 2) out vec3 outDiffuse_;
        layout (location = 3) out vec3 outNormal_;

		void main()
		{
            outPosition_        = v_out_.mMPos;

            outSpecular_.rgb    = texture(material_.mSpecular, v_out_.mUV).rgb;
            outSpecular_.a      = material_.mShininess;

            outDiffuse_         = texture(material_.mDiffuse0, v_out_.mUV).rgb;

            outNormal_          = vec3(texture(material_.mNormal, v_out_.mUV));
		    outNormal_          = v_out_.mTBN * normalize(outNormal_ *2 - 1.0);
		}
	End Fragment
End Pass