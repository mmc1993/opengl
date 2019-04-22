Pass
	CullFace Front
	DepthTest
	DepthWrite
	RenderType Shadow
	DrawType Index

	Vertex
		#version 440 core

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
		#version 410 core

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

                }
                break;
            case LIGHT_TYPE_POINT_:
                {

                }
                break;
            case LIGHT_TYPE_SPOT_:
                {

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
	RenderType Forward
	DrawType Index

	Vertex
		#version 440 core

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
			mat3 mTBNR;
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
			v_out_.mTBNR = transpose(v_out_.mTBN);

			v_out_.mUV = a_uv_;

			gl_Position = v_out_.mMVPPos;
		}
	End Vertex

	Fragment
		#version 410 core

		struct LightDirectParam_ {
			int mSMP;
			mat4 mMatrix;
			vec3 mNormal;
			vec3 mAmbient;
			vec3 mDiffuse;
			vec3 mSpecular;
			vec3 mPosition;
		};

        struct LightPointParam_ {
            int mSMP;
            float mK0, mK1, mK2;
            vec3 mAmbient;
            vec3 mDiffuse;
            vec3 mSpecular;
            vec3 mPosition;
        };

		struct LightSpotParam_ {
			int mSMP;	
			float mK0;
            float mK1;
            float mK2;
			float mInCone;
            float mOutCone;
			vec3 mNormal;
			vec3 mAmbient;
			vec3 mDiffuse;
			vec3 mSpecular;
			vec3 mPosition;
		};

		layout (std140) uniform LightDirect_ {
			LightDirectParam_ mParam[2];
		} light_direct_;

        layout (std140) uniform LightPoint_ {
            LightPointParam_ mParam[4];
        } light_point_;

		layout (std140) uniform LightSpot_ {
			LightSpotParam_ mParam[4];
		} light_spot_;

        uniform samplerCubeArray shadow_map_3d_;
        uniform sampler2DArray shadow_map_2d_;
		uniform int light_count_direct_;
        uniform int light_count_point_;
		uniform int light_count_spot_;
		uniform vec3 camera_pos_;
		uniform vec3 camera_eye_;

		uniform struct Material_ {
			float mShininess;
			sampler2D mNormal;
			sampler2D mSpecular;
			sampler2D mDiffuse0;
			sampler2D mDiffuse1;
			sampler2D mDiffuse2;
			sampler2D mDiffuse3;
		} material_;

		in V_OUT_{
			vec3 mNormal;
			vec4 mMVPPos;
			vec3 mMVPos;
			vec3 mMPos;
			mat3 mTBNR;
			mat3 mTBN;
			vec2 mUV;
		} v_out_;

		out vec4 color_;

        int CheckInView(vec4 vec)
        {
            int ret = 0;
            if		(vec.x < -vec.w) ret |= 1;
            else if (vec.x >  vec.w) ret |= 2;
            else if (vec.y < -vec.w) ret |= 4;
            else if (vec.y >  vec.w) ret |= 8;
            else if (vec.z < -vec.w) ret |= 16;
            else if (vec.z >  vec.w) ret |= 32;
            return ret;
        }

        float CalculateDirectShadow(const LightDirectParam_ lightParam)
        {
            vec4 position = lightParam.mMatrix * vec4(v_out_.mMPos, 1);
            if (CheckInView(position) != 0) { return 0; }

            float shadow = 0.0f;
            position.xyz = position.xyz / position.w * 0.5f + 0.5f;
            float zorder = position.z;
            position.z   = lightParam.mSMP;
            vec3 texstep = 1.0f / textureSize(shadow_map_2d_, 0);
            float depth = texture(shadow_map_2d_, position.xyz + vec3(-texstep.x,  texstep.y, 0)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_2d_, position.xyz + vec3( 0,          texstep.y, 0)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_2d_, position.xyz + vec3( texstep.x,  texstep.y, 0)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_2d_, position.xyz + vec3(-texstep.x,  0		, 0)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_2d_, position.xyz + vec3( 0,          0	    , 0)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_2d_, position.xyz + vec3( texstep.x,  0	    , 0)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_2d_, position.xyz + vec3(-texstep.x, -texstep.y, 0)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_2d_, position.xyz + vec3( 0,		  -texstep.y, 0)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_2d_, position.xyz + vec3( texstep.x, -texstep.y, 0)).r; shadow += zorder < depth? 1: 0;
            return shadow / 9.0f;
        }

		//	计算漫反射缩放因子
		float CalculateDiffuseScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal)
		{
			return max(dot(fragNormal, lightNormal), 0);
		}

		//	计算镜面反射缩放因子
		float CalculateSpecularScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal)
		{
			vec3 h = (lightNormal + cameraNormal) * 0.5f;
			return pow(max(dot(fragNormal, h), 0), material_.mShininess);
		}

		//	计算距离衰减缩放因子
		float CalculateDistanceScale(vec3 fragPosition, vec3 lightPosition, float k0, float k1, float k2)
		{
			float len = length(fragPosition - lightPosition);
			return 1.0f / (k0 + k1 * len + k2 * len * len);
		}

		//	计算外锥衰减缩放因子
		float CalculateOutConeScale(float inCone, float outCone, vec3 lightNormal, vec3 lightToFrag)
		{
			float cone = dot(lightNormal, lightToFrag);
			return clamp((cone - outCone) / (inCone - outCone), 0, 1);
		}

		vec3 CalculateDirect(const LightDirectParam_ lightParam, vec3 fragNormal, vec3 cameraNormal, vec2 uv)
		{
            float shadow = CalculateDirectShadow(lightParam);

			float diff = CalculateDiffuseScale(fragNormal, -lightParam.mNormal, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, -lightParam.mNormal, cameraNormal);

			vec3 ambient = lightParam.mAmbient * texture(material_.mDiffuse0, uv).rgb;
			vec3 diffuse = lightParam.mDiffuse * texture(material_.mDiffuse0, uv).rgb * diff;
			vec3 specular = lightParam.mSpecular * texture(material_.mSpecular, uv).rgb * spec;
			return ambient + (diffuse + specular) * shadow;
		}

        vec3 CalculatePoint(const LightPointParam_ lightParam, vec3 fragNormal, vec3 cameraNormal, vec2 uv)
        {
            vec3 fragToLight = normalize(lightParam.mPosition - v_out_.mMPos);

			float diff = CalculateDiffuseScale(fragNormal, fragToLight, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, fragToLight, cameraNormal);

			vec3 ambient = lightParam.mAmbient * texture(material_.mDiffuse0, uv).rgb;
			vec3 diffuse = lightParam.mDiffuse * texture(material_.mDiffuse0, uv).rgb * diff;
			vec3 specular = lightParam.mSpecular * texture(material_.mSpecular, uv).rgb * spec;

			//	距离衰减
			float distance = CalculateDistanceScale(v_out_.mMPos, 
                                                    lightParam.mPosition, 
                                                    lightParam.mK0, 
                                                    lightParam.mK1, 
                                                    lightParam.mK2);

			return (ambient + diffuse + specular) * distance;
        }

		vec3 CalculateSpot(const LightSpotParam_ lightParam, vec3 fragNormal, vec3 cameraNormal, vec2 uv)
		{
			vec3 fragToLight = normalize(lightParam.mPosition - v_out_.mMPos);

			float diff = CalculateDiffuseScale(fragNormal, fragToLight, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, fragToLight, cameraNormal);

			vec3 ambient = lightParam.mAmbient * texture(material_.mDiffuse0, uv).rgb;
			vec3 diffuse = lightParam.mDiffuse * texture(material_.mDiffuse0, uv).rgb * diff;
			vec3 specular = lightParam.mSpecular * texture(material_.mSpecular, uv).rgb * spec;

			//	光锥衰减
			float weight = CalculateOutConeScale(lightParam.mInCone, 
                                                 lightParam.mOutCone, 
                                                 lightParam.mNormal, -fragToLight);

			//	距离衰减
			float distance = CalculateDistanceScale(v_out_.mMPos, 
                                                    lightParam.mPosition, 
                                                    lightParam.mK0, 
                                                    lightParam.mK1, 
                                                    lightParam.mK2);

			return (ambient + diffuse + specular) * weight * distance;
		}

		void main()
		{
			vec3 cameraNormal = normalize(camera_pos_ - v_out_.mMPos);

			vec3 fragNormal = vec3(texture(material_.mNormal, v_out_.mUV));
				 fragNormal = v_out_.mTBN * normalize(fragNormal *2 - 1.0);

			vec3 outColor = vec3(0, 0, 0);
			for (int i = 0; i != light_count_direct_; ++i)
			{
				outColor += CalculateDirect(light_direct_.mParam[i], fragNormal, cameraNormal, v_out_.mUV);
			}

            for (int i = 0; i != light_count_point_; ++i)
            {
                outColor += CalculatePoint(light_point_.mParam[i], fragNormal, cameraNormal, v_out_.mUV);
            }

			for (int i = 0; i != light_count_spot_; ++i)
			{
				outColor += CalculateSpot(light_spot_.mParam[i], fragNormal, cameraNormal, v_out_.mUV);
			}

			color_ = vec4(outColor, 1.0f);
		}
	End Fragment
End Pass