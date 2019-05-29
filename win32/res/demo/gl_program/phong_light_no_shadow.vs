void PhongLightNoShadowVS()
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