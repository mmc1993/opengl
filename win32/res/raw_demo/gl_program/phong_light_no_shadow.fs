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

vec3 CalculateDirect(const int i, vec3 fragNormal, vec3 cameraNormal, vec2 uv)
{
    float diff = CalculateDiffuseScale(fragNormal, -light_direct_.mParam[i].mNormal, cameraNormal);
    float spec = CalculateSpecularScale(fragNormal, -light_direct_.mParam[i].mNormal, cameraNormal);

    vec3 ambient = light_direct_.mParam[i].mAmbient * texture(material_.mDiffuse0, uv).rgb;
    vec3 diffuse = light_direct_.mParam[i].mDiffuse * texture(material_.mDiffuse0, uv).rgb * diff;
    vec3 specular = light_direct_.mParam[i].mSpecular * texture(material_.mSpecular, uv).rgb * spec;
    return ambient + (diffuse + specular);
}

vec3 CalculatePoint(const int i, vec3 fragNormal, vec3 cameraNormal, vec2 uv)
{
    vec3 fragToLight = normalize(light_point_.mParam[i].mPosition - v_out_.mMPos);

    float diff = CalculateDiffuseScale(fragNormal, fragToLight, cameraNormal);
    float spec = CalculateSpecularScale(fragNormal, fragToLight, cameraNormal);

    vec3 ambient = light_point_.mParam[i].mAmbient * texture(material_.mDiffuse0, uv).rgb;
    vec3 diffuse = light_point_.mParam[i].mDiffuse * texture(material_.mDiffuse0, uv).rgb * diff;
    vec3 specular = light_point_.mParam[i].mSpecular * texture(material_.mSpecular, uv).rgb * spec;

    //	距离衰减
    float distance = CalculateDistanceScale(v_out_.mMPos, 
                                            light_point_.mParam[i].mPosition, 
                                            light_point_.mParam[i].mK0, 
                                            light_point_.mParam[i].mK1,
                                            light_point_.mParam[i].mK2);

    return (ambient + (diffuse + specular)) * distance;
}

vec3 CalculateSpot(const int i, vec3 fragNormal, vec3 cameraNormal, vec2 uv)
{
    vec3 fragToLight = normalize(light_spot_.mParam[i].mPosition - v_out_.mMPos);

    float diff = CalculateDiffuseScale(fragNormal, fragToLight, cameraNormal);
    float spec = CalculateSpecularScale(fragNormal, fragToLight, cameraNormal);

    vec3 ambient = light_spot_.mParam[i].mAmbient * texture(material_.mDiffuse0, uv).rgb;
    vec3 diffuse = light_spot_.mParam[i].mDiffuse * texture(material_.mDiffuse0, uv).rgb * diff;
    vec3 specular = light_spot_.mParam[i].mSpecular * texture(material_.mSpecular, uv).rgb * spec;

    //	光锥衰减
    float weight = CalculateOutConeScale(light_spot_.mParam[i].mInCone, 
                                            light_spot_.mParam[i].mOutCone, 
                                            light_spot_.mParam[i].mNormal, -fragToLight);

    //	距离衰减
    float distance = CalculateDistanceScale(v_out_.mMPos, 
                                            light_spot_.mParam[i].mPosition, 
                                            light_spot_.mParam[i].mK0, 
                                            light_spot_.mParam[i].mK1, 
                                            light_spot_.mParam[i].mK2);

    return (ambient + (diffuse + specular)) * weight * distance;
}

void PhongLightNoShadowFS()
{
    vec3 cameraNormal = normalize(camera_pos_ - v_out_.mMPos);

    vec3 fragNormal = vec3(texture(material_.mNormal, v_out_.mUV));
            fragNormal = v_out_.mTBN * normalize(fragNormal *2 - 1.0);

    vec3 outColor = vec3(0, 0, 0);
    for (int i = 0; i != light_count_direct_; ++i)
    {
        outColor += CalculateDirect(i, fragNormal, cameraNormal, v_out_.mUV);
    }

    for (int i = 0; i != light_count_point_; ++i)
    {
        outColor += CalculatePoint(i, fragNormal, cameraNormal, v_out_.mUV);
    }

    for (int i = 0; i != light_count_spot_; ++i)
    {
        outColor += CalculateSpot(i, fragNormal, cameraNormal, v_out_.mUV);
    }

    color_ = vec4(outColor, 1.0f);
}