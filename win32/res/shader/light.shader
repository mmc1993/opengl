Pass
	CullFace Front
	DepthTest
	DepthWrite
	RenderType Light
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
            int mSMP;
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

		void main()
		{
            gl_FragColor = vec4(0, 0, 0, 0);
		}
	End Fragment
End Pass