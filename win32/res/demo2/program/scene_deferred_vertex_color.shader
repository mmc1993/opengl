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
		layout(location = 2) in vec3 a_n_;
		layout(location = 5) in vec4 a_c_;

		uniform mat4 matrix_mvp_;
		uniform mat4 matrix_m_;
		uniform mat3 matrix_n_;

		out V_OUT_ {
			vec3 mNormal;
			vec3 mMPos;
			vec4 mColor;
		} v_out_;

		void main()
		{
			vec4 apos = vec4(a_pos_, 1);
			v_out_.mColor 	= a_c_;
			v_out_.mMPos 	= vec3(matrix_m_ * apos);
			v_out_.mNormal 	= vec3(matrix_n_ * a_n_);
			gl_Position = vec4(matrix_mvp_ * apos);
		}
	End Vertex

	Fragment
		#version 330 core

		in V_OUT_{
			vec3 mNormal;
			vec3 mMPos;
			vec4 mColor;
		} v_out_;

        layout (location = 0) out vec3 outPosition_;
        layout (location = 1) out vec4 outSpecular_;
        layout (location = 2) out vec3 outDiffuse_;
        layout (location = 3) out vec3 outNormal_;

		void main()
		{
            outPosition_        = v_out_.mMPos;

            outSpecular_.rgb    = v_out_.mColor.rgb;
            outSpecular_.a      = 32;

            outDiffuse_         = v_out_.mColor.rgb;

            outNormal_          = v_out_.mNormal;
		}
	End Fragment
End Pass