
void mmcLight()
{
	color_ *= light_.mAmbient;
}

void main()
{
	main_();
	mmcLight();
}