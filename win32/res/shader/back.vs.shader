
void main()
{
	main_();
	vuv_ = uv_;
	vpos_mv_ = mv_ * vec4(pos_, 1.0);
	vpos_mvp_ = mvp_ * vec4(pos_, 1.0);
}