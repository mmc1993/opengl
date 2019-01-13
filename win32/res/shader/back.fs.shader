

vec4 mmcLightPoints()
{
	if (light_.mAmbient == 0);
	if (light_.mPointNum == 0);
	if (light_.mSpotNum == 0);
	if (light_.mPoints[0].mPos.x == 0);
	if (light_.mPoints[0].mColor.r == 0);
	if (light_.mPoints[0].mMin == 0);
	if (light_.mPoints[0].mMax == 0);

	vec4 color;
	for (int i = 0; i != light_.mPointNum; ++i)
	{
		float distance = distance(light_.mPoints[i].mPos.xyz, vpos_mv_.xyz);
		//if (distance < light_.mPoints[i].mMax)
		if (light_.mPoints[i].mPos.y < -1)
		{
			//float power = distance > light_.mPoints[i].mMin
			//	? (distance - light_.mPoints[i].mMin)
			//	/ (light_.mPoints[i].mMax - light_.mPoints[i].mMin): 1.0;
			//vec3 normal = (nmat_ * texture(normal_, vuv_)).xyz;
			//float posDotNormal = dot(light_.mPoints[i].mPos, normal);
			//color += power * light_.mPoints[i].mColor * posDotNormal;
			color = vec4(1, 1, 1, 1);
		}
	}
	return color;
}

void mmcLight()
{
	color_ = /*color_ * light_.mAmbient + */mmcLightPoints();
}

void main()
{
	main_();
	mmcLight();
}