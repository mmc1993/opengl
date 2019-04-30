Pass
	CullFace Back
	RenderQueue Geometric
	RenderType Deferred
	DrawType Index

	Vertex
		#version 440 core

		layout(location = 0) in vec3 a_pos_;

		uniform vec3 camera_pos_;
		uniform mat4 matrix_mvp_;

		void main()
		{
			gl_Position  = vec4(matrix_mvp_ * vec4(a_pos_, 1));
		}
	End Vertex

	Fragment
		#version 440 core

		uniform vec3 camera_pos_;

		uniform struct Material_ {
			sampler2D mNormal;
			sampler2D mSpecular;
			sampler2D mDiffuse0;
			sampler2D mPosition;
		} material_;

        out vec4 color_;

		void main()
		{
            color_ = vec4(0, 0, 0, 0);
		}
	End Fragment
End Pass