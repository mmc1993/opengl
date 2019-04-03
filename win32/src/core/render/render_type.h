#pragma once

#include "../include.h"

class RenderMatrix {
public:
	enum ModeType { kPROJECT, kVIEW, kMODEL, };

public:
	RenderMatrix()
	{ }

	~RenderMatrix()
	{ }

	void Pop(ModeType mode)
	{
		GetStack(mode).pop();
	}

	void Push(ModeType mode)
	{
		GetStack(mode).push(GetStack(mode).top());
	}

	void Identity(ModeType mode)
	{
		GetStack(mode).push(glm::mat4(1));
	}

	void Mul(ModeType mode, const glm::mat4 & mat)
	{
		GetStack(mode).top() *= mat;
	}

	const glm::mat4 & Top(ModeType mode) const
	{
		return GetStack(mode).top();
	}

	const glm::mat4 & GetM() const
	{
		return Top(ModeType::kMODEL);
	}

	const glm::mat4 & GetV() const
	{
		return Top(ModeType::kVIEW);
	}

	const glm::mat4 & GetP() const
	{
		return Top(ModeType::kPROJECT);
	}

private:
	std::stack<glm::mat4> & GetStack(ModeType mode)
	{
		return _RenderMatrixs.at((size_t)mode);
	}

	const std::stack<glm::mat4> & GetStack(ModeType mode) const
	{
		return const_cast<RenderMatrix *>(this)->GetStack(mode);
	}

private:
	std::array<std::stack<glm::mat4>, 3> _RenderMatrixs;
};

enum RenderQueue {
    kGEOMETRIC,     //  ≥£πÊªÊ÷∆
    kOPCITY,        //  Õ∏√˜ªÊ÷∆
    kUI,            //  ∂•≤„ªÊ÷∆
};

enum RenderType {
    kSHADOW,        //  ∫Ê≈‡“ı”∞Ã˘Õº
    kFORWARD,       //  ’˝œÚ‰÷»æ
    kDEFERRED,      //  —”≥Ÿ‰÷»æ
};