
void mmcLightPoints()
{
	for (int i = 0; i != light_.mPointNum; ++i)
	{

	}
}

void mmcLight()
{
	color_ *= light_.mAmbient;
}

void main()
{
	main_();
	mmcLight();
}