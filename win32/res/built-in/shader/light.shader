Pass
	CullFace Front
    Blend One One
	RenderType Light
	DrawType Index

	Vertex
		#version 330 core

		layout(location = 0) in vec3 a_pos_;

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

		uniform mat4 matrix_mvp_;

        uniform int light_type_;

		out V_OUT_ {
            vec4 mMVPPos;
		} v_out_;

        #define LIGHT_TYPE_DIRECT_  0
        #define LIGHT_TYPE_POINT_   1
        #define LIGHT_TYPE_SPOT_    2

		void main()
		{
            switch (light_type_)
            {
            case LIGHT_TYPE_DIRECT_:
                gl_Position = vec4(a_pos_, 1);
                v_out_.mMVPPos = gl_Position;
                break;
            case LIGHT_TYPE_POINT_:
                gl_Position = matrix_mvp_ * vec4(a_pos_, 1);
                v_out_.mMVPPos = gl_Position;
                break;
            case LIGHT_TYPE_SPOT_:
                vec3 normal = a_pos_ - vec3(0, 0, 1);
                vec3 offset = normal * (light_spot_.mOutCone - 0.5f);
                vec3 point = a_pos_ + offset;

                gl_Position = matrix_mvp_ * vec4(point, 1);
                v_out_.mMVPPos = gl_Position;
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

        uniform sampler2D   shadow_map_direct_0_;
        uniform samplerCube shadow_map_point_0_;
        uniform sampler2D   shadow_map_spot_0_;
        uniform int light_type_;

		uniform vec3 camera_pos_;

        #define LIGHT_TYPE_DIRECT_  0
        #define LIGHT_TYPE_POINT_   1
        #define LIGHT_TYPE_SPOT_    2

        in V_OUT_ {
            vec4 mMVPPos;
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

        float CalculateDirectShadow(const vec3 position)
        {
            vec4 pos = light_direct_.mMatrix * vec4(position, 1);
            if (CheckInView(pos) != 0) { return 0; }
            pos.xyz = pos.xyz / pos.w * 0.5f + 0.5f;
			float zorder = pos.z;
            float shadow = 0.0f;
            vec2 texstep = 1.0f / textureSize(shadow_map_direct_0_, 0);
            float depth = texture(shadow_map_direct_0_, pos.xy + vec2(-texstep.x,  texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_direct_0_, pos.xy + vec2( 0,          texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_direct_0_, pos.xy + vec2( texstep.x,  texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_direct_0_, pos.xy + vec2(-texstep.x,  0		)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_direct_0_, pos.xy + vec2( 0,          0	    )).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_direct_0_, pos.xy + vec2( texstep.x,  0	    )).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_direct_0_, pos.xy + vec2(-texstep.x, -texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_direct_0_, pos.xy + vec2( 0,		  -texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_direct_0_, pos.xy + vec2( texstep.x, -texstep.y)).r; shadow += zorder < depth? 1: 0;
            return shadow / 9.0f;
        }

		float CalculateSpotShadow(const vec3 position)
		{
			vec4 pos = light_spot_.mMatrix * vec4(position, 1);
            if (CheckInView(pos) != 0) { return 0; }
            pos.xyz = pos.xyz / pos.w * 0.5f + 0.5f;
			float zorder = pos.z;
            float shadow = 0.0f;
            vec2 texstep = 1.0f / textureSize(shadow_map_spot_0_, 0);
            float depth = texture(shadow_map_spot_0_, pos.xy + vec2(-texstep.x,  texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_spot_0_, pos.xy + vec2( 0,          texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_spot_0_, pos.xy + vec2( texstep.x,  texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_spot_0_, pos.xy + vec2(-texstep.x,  0        )).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_spot_0_, pos.xy + vec2( 0,          0        )).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_spot_0_, pos.xy + vec2( texstep.x,  0        )).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_spot_0_, pos.xy + vec2(-texstep.x, -texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_spot_0_, pos.xy + vec2( 0,	        -texstep.y)).r; shadow += zorder < depth? 1: 0;
                  depth = texture(shadow_map_spot_0_, pos.xy + vec2( texstep.x, -texstep.y)).r; shadow += zorder < depth? 1: 0;
            return shadow / 9.0f;
		}

		float CalculatePointShadow(const vec3 position)
		{
			vec3 normal 	= position - light_point_.mPosition;
			float zorder 	= texture(shadow_map_point_0_, normal).r;
			return length(normal) > zorder * light_point_.mFar? 0: 1;
		}

		//	计算漫反射缩放因子
		float CalculateDiffuseScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal)
		{
			return max(dot(fragNormal, lightNormal), 0);
		}

		//	计算镜面反射缩放因子
		float CalculateSpecularScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal, float shininess)
		{
			vec3 h = (lightNormal + cameraNormal) * 0.5f;
			return pow(max(dot(fragNormal, h), 0), shininess);
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

		vec3 CalculateDirect(vec3 fragCoord, vec3 fragNormal, vec3 cameraNormal, vec3 diffuse, vec4 specular)
		{
			float shadow = CalculateDirectShadow(fragCoord);
			float diff = CalculateDiffuseScale(fragNormal, -light_direct_.mNormal, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, -light_direct_.mNormal, cameraNormal, specular.a);

			return diffuse * light_direct_.mAmbient
                + (diffuse * light_direct_.mDiffuse * diff
                + specular.rgb * light_direct_.mSpecular * spec) * shadow;
		}

        vec3 CalculatePoint(vec3 fragCoord, vec3 fragNormal, vec3 cameraNormal, vec3 diffuse, vec4 specular)
        {
			float shadow = CalculatePointShadow(fragCoord);
            vec3 fragToLight = normalize(light_point_.mPosition - fragCoord);
			float diff = CalculateDiffuseScale(fragNormal, fragToLight, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, fragToLight, cameraNormal, specular.w);

			//	距离衰减
			float distance = CalculateDistanceScale(fragCoord, 
													light_point_.mPosition, 
													light_point_.mK0, 
													light_point_.mK1,
													light_point_.mK2);

			return (diffuse * light_point_.mAmbient 
                + (diffuse * light_point_.mDiffuse * diff 
                + specular.rgb * light_point_.mSpecular * spec) * shadow) * distance;
        }

		vec3 CalculateSpot(vec3 fragCoord, vec3 fragNormal, vec3 cameraNormal, vec3 diffuse, vec4 specular)
		{
			float shadow = CalculateSpotShadow(fragCoord);
			vec3 fragToLight = normalize(light_spot_.mPosition - fragCoord);
			float diff = CalculateDiffuseScale(fragNormal, fragToLight, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, fragToLight, cameraNormal, specular.w);

			//	光锥衰减
			float weight = CalculateOutConeScale(light_spot_.mInCone, 
                                                 light_spot_.mOutCone, 
                                                 light_spot_.mNormal, -fragToLight);

			//	距离衰减
			float distance = CalculateDistanceScale(fragCoord, 
                                                    light_spot_.mPosition, 
                                                    light_spot_.mK0, 
                                                    light_spot_.mK1, 
                                                    light_spot_.mK2);

			return (diffuse * light_spot_.mAmbient 
                + (diffuse * light_spot_.mDiffuse * diff 
                + specular.rgb * light_spot_.mSpecular * spec) * shadow) * weight * distance;
		}

		void main()
		{
            vec2 uv = v_out_.mMVPPos.xy / v_out_.mMVPPos.w * 0.5 + 0.5;

            vec3 position = vec3(texture(gbuffer_position_, uv));
            vec4 specular = vec4(texture(gbuffer_specular_, uv));
            vec3 diffuse = vec3(texture(gbuffer_diffuse_, uv));
            vec3 normal = vec3(texture(gbuffer_normal_, uv));

            switch (light_type_)
            {
            case LIGHT_TYPE_DIRECT_:
                color_ = vec4(CalculateDirect(position, normal, normalize(camera_pos_ - position), diffuse, specular), 1);
                break;
            case LIGHT_TYPE_POINT_:
                color_ = vec4(CalculatePoint(position, normal, normalize(camera_pos_ - position), diffuse, specular), 1);
                break;
            case LIGHT_TYPE_SPOT_:
                color_ = vec4(CalculateSpot(position, normal, normalize(camera_pos_ - position), diffuse, specular), 1);
                break;
            }
		}
	End Fragment
End Pass

Pass
	CullFace Front
    Blend One One
	RenderType Light
	DrawType Index

	Vertex
		#version 330 core

		layout(location = 0) in vec3 a_pos_;

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

		uniform mat4 matrix_mvp_;

        uniform int light_type_;

		out V_OUT_ {
            vec4 mMVPPos;
		} v_out_;

        #define LIGHT_TYPE_DIRECT_  0
        #define LIGHT_TYPE_POINT_   1
        #define LIGHT_TYPE_SPOT_    2

		void main()
		{
            switch (light_type_)
            {
            case LIGHT_TYPE_DIRECT_:
                gl_Position = vec4(a_pos_, 1);
                v_out_.mMVPPos = gl_Position;
                break;
            case LIGHT_TYPE_POINT_:
                gl_Position = matrix_mvp_ * vec4(a_pos_, 1);
                v_out_.mMVPPos = gl_Position;
                break;
            case LIGHT_TYPE_SPOT_:
                vec3 normal = a_pos_ - vec3(0, 0, 1);
                vec3 offset = normal * (light_spot_.mOutCone - 0.5f);
                vec3 point = a_pos_ + offset;

                gl_Position = matrix_mvp_ * vec4(point, 1);
                v_out_.mMVPPos = gl_Position;
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

        uniform int light_type_;

		uniform vec3 camera_pos_;

        #define LIGHT_TYPE_DIRECT_  0
        #define LIGHT_TYPE_POINT_   1
        #define LIGHT_TYPE_SPOT_    2

        in V_OUT_ {
            vec4 mMVPPos;
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

		//	计算漫反射缩放因子
		float CalculateDiffuseScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal)
		{
			return max(dot(fragNormal, lightNormal), 0);
		}

		//	计算镜面反射缩放因子
		float CalculateSpecularScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal, float shininess)
		{
			vec3 h = (lightNormal + cameraNormal) * 0.5f;
			return pow(max(dot(fragNormal, h), 0), shininess);
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

		vec3 CalculateDirect(vec3 fragCoord, vec3 fragNormal, vec3 cameraNormal, vec3 diffuse, vec4 specular)
		{
			float diff = CalculateDiffuseScale(fragNormal, -light_direct_.mNormal, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, -light_direct_.mNormal, cameraNormal, specular.a);

			return diffuse * light_direct_.mAmbient
                + (diffuse * light_direct_.mDiffuse * diff
                + specular.rgb * light_direct_.mSpecular * spec);
		}

        vec3 CalculatePoint(vec3 fragCoord, vec3 fragNormal, vec3 cameraNormal, vec3 diffuse, vec4 specular)
        {
            vec3 fragToLight = normalize(light_point_.mPosition - fragCoord);
			float diff = CalculateDiffuseScale(fragNormal, fragToLight, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, fragToLight, cameraNormal, specular.w);

			//	距离衰减
			float distance = CalculateDistanceScale(fragCoord, 
													light_point_.mPosition, 
													light_point_.mK0, 
													light_point_.mK1,
													light_point_.mK2);

			return (diffuse * light_point_.mAmbient 
                + (diffuse * light_point_.mDiffuse * diff 
                + specular.rgb * light_point_.mSpecular * spec)) * distance;
        }

		vec3 CalculateSpot(vec3 fragCoord, vec3 fragNormal, vec3 cameraNormal, vec3 diffuse, vec4 specular)
		{
			vec3 fragToLight = normalize(light_spot_.mPosition - fragCoord);
			float diff = CalculateDiffuseScale(fragNormal, fragToLight, cameraNormal);
			float spec = CalculateSpecularScale(fragNormal, fragToLight, cameraNormal, specular.w);

			//	光锥衰减
			float weight = CalculateOutConeScale(light_spot_.mInCone, 
                                                 light_spot_.mOutCone, 
                                                 light_spot_.mNormal, -fragToLight);

			//	距离衰减
			float distance = CalculateDistanceScale(fragCoord, 
                                                    light_spot_.mPosition, 
                                                    light_spot_.mK0, 
                                                    light_spot_.mK1, 
                                                    light_spot_.mK2);

			return (diffuse * light_spot_.mAmbient 
                + (diffuse * light_spot_.mDiffuse * diff 
                + specular.rgb * light_spot_.mSpecular * spec)) * weight * distance;
		}

		void main()
		{
            vec2 uv = v_out_.mMVPPos.xy / v_out_.mMVPPos.w * 0.5 + 0.5;

            vec3 position = vec3(texture(gbuffer_position_, uv));
            vec4 specular = vec4(texture(gbuffer_specular_, uv));
            vec3 diffuse = vec3(texture(gbuffer_diffuse_, uv));
            vec3 normal = vec3(texture(gbuffer_normal_, uv));

            switch (light_type_)
            {
            case LIGHT_TYPE_DIRECT_:
                color_ = vec4(CalculateDirect(position, normal, normalize(camera_pos_ - position), diffuse, specular), 1);
                break;
            case LIGHT_TYPE_POINT_:
                color_ = vec4(CalculatePoint(position, normal, normalize(camera_pos_ - position), diffuse, specular), 1);
                break;
            case LIGHT_TYPE_SPOT_:
                color_ = vec4(CalculateSpot(position, normal, normalize(camera_pos_ - position), diffuse, specular), 1);
                break;
            }
		}
	End Fragment
End Pass