// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef VERTEX_MATH
#define VERTEX_MATH

#include <iostream>
#include <cmath>
#include <ostream>

#define PI 3.14159265

template <class T2, int var2>
class Vector;

template <class T, int var>
class Matrix
{
	T m_matrix[var][var] {};
public:
	Matrix(T arg = 0)
	{
		for(int i = 0; i < var; ++i)
		{
			m_matrix[i][i] = arg;
			// if(i == (var-1))
			// 	m_matrix[i][i] = 1;
		}
  	}

    Matrix(Vector<T, var> _vector1, Vector<T, var> _vector2,
           Vector<T, var> _vector3, Vector<T, var> _vector4)
    {
        m_matrix[0][0] = _vector1[0];
        m_matrix[0][1] = _vector1[1];
        m_matrix[0][2] = _vector1[2];
        m_matrix[0][3] = _vector1[3];

        m_matrix[1][0] = _vector2[0];
        m_matrix[1][1] = _vector2[1];
        m_matrix[1][2] = _vector2[2];
        m_matrix[1][3] = _vector2[3];

        m_matrix[2][0] = _vector3[0];
        m_matrix[2][1] = _vector3[1];
        m_matrix[2][2] = _vector3[2];
        m_matrix[2][3] = _vector3[3];

        m_matrix[3][0] = _vector4[0];
        m_matrix[3][1] = _vector4[1];
        m_matrix[3][2] = _vector4[2];
        m_matrix[3][3] = _vector4[3];
    }

    void SelfTensorTranspose()
    {
        T tempMatrix[var][var];
        for(int p = 0; p < var; ++p)
            for(int u = 0; u < var; ++u)
            {
                tempMatrix[p][u] = m_matrix[u][p];
            }
        for(int j = 0; j < var; ++j)
            for(int z = 0; z < var; ++z)
            {
                this->m_matrix[j][z] = tempMatrix[j][z];
            }
    }

    void SelfIdentity()
    {
        for(int i = 0; i < var; ++i)
            for(int j = 0; j < var; ++j)
            {
                if(i == j)
                    this->m_matrix[i][j] = 1.0f;
                else
                    this->m_matrix[i][j] = 0.0f;
            }
    }

	Matrix<T, var> operator+(Matrix matrix);
	Matrix<T, var> operator*(T scalar);
	Matrix<T, var> operator*(Matrix& matrix);
	T* operator[](const int index);
	const T* operator[](const int index) const;
	template<class T2, int var2>
	Vector<T2, var2> operator*(Vector<T2, var2>& vector);
};

typedef Vector<float, 1> vec1;
typedef Vector<float, 2> vec2;
typedef Vector<float, 3> vec3;
typedef Vector<float, 4> vec4;

typedef Matrix<float, 1> mat1;
typedef Matrix<float, 2> mat2;
typedef Matrix<float, 3> mat3;
typedef Matrix<float, 4> mat4;

template <class T, int var>
Matrix<T, var> Matrix<T, var>::operator+(Matrix matrix) {
	Matrix<T, var> tempMatrix;
	for(int i = 0; i < var; ++i)
		for(int j = 0; j < var; ++j)
			tempMatrix[i][j] = this->m_matrix[i][j] + matrix.m_matrix[i][j];

	return tempMatrix;
}
	
template <class T, int var>
Matrix<T, var> Matrix<T, var>::operator*(T scalar) {
	Matrix<T, var> tempMatrix;
	for(int i = 0; i < var; ++i)
		for(int j = 0; j < var; ++j)
			tempMatrix[i][j] = this->m_matrix[i][j] * scalar;

	return tempMatrix;
}

template<class T, int var>
Matrix<T, var> Matrix<T, var>::operator*(Matrix& matrix)
{
	Matrix<T, var> tempMatrix;
	for(int i = 0; i < var; ++i)
		for(int j = 0; j < var; ++j)
			for(int n = 0; n < var; ++n)
			{
				tempMatrix.m_matrix[i][j] += m_matrix[i][n] * matrix.m_matrix[n][j];
			}
	return tempMatrix;
}

template <class T, int var>
T* Matrix<T, var>::operator[](const int index)
{
	return m_matrix[index];
}

template <class T, int var>
const T* Matrix<T, var>::operator[](const int index) const
{
	return m_matrix[index];
}

template <int var2>
std::ostream& operator<<(std::ostream& ostream, const Matrix<float, var2>& matrix) {
	for ( int i = 0; i < var2; ++i) {
		ostream << std::endl;
		for ( int j = 0; j < var2; ++j) {
			ostream << matrix[i][j] << " ";
		}
	}
	return ostream;
}

template <class T, int var>
template <class T2, int var2>
Vector<T2, var2> Matrix<T, var>::operator*(Vector<T2, var2>& vector)
{
	static_assert(var == var2, "Size error");
	Vector<T2, var2> tempVector;
	for(int i = 0; i < var2; ++i)
		for(int j = 0; j < var; ++j)
		{
			tempVector[i] += m_matrix[i][j] * vector[j];
		}
	return tempVector;
}

template <class T2, int var2>
class Vector
{
	public: T2 m_vector[var2] {};
public:
	Vector(T2 arg1 = 0, T2 arg2 = 0, T2 arg3 = 0, T2 arg4 = 0)
	{
		T2 array[4] = {arg1, arg2, arg3, arg4};
		for(int i = 0; i < var2; ++i)
		{
			m_vector[i] = array[i];
		}
	}

    T2& operator[](const int index);
	const T2& operator[](const int index) const;
	template<class T, int var>
	Vector<T2, var2> operator*(const Matrix<T, var>& matrix);
    Vector<T2, var2> operator*(Vector<T2, var2> _vector);
    Vector<T2, var2> operator*=(Vector<T2, var2> _vector);
    Vector<T2, var2> operator-(Vector<T2, var2> _vector);
    Vector<T2, var2> operator+(Vector<T2, var2> _vector);
    void operator-=(Vector<T2, var2> _vector);
    void operator+=(Vector<T2, var2> _vector);
    Vector<T2, var2> operator*(T2 _scalar);
	Vector<T2, var2> operator-();
	T2 Length() const;
};

template <class T2, int var2>
T2 Vector<T2, var2>::Length() const { 
	return std::sqrt(m_vector[0] * m_vector[0] + m_vector[1] * m_vector[1] + m_vector[2] * m_vector[2]);		
}

template <class T2, int var2>
std::ostream& operator<<(std::ostream& ostream, const Vector<T2, var2>& vector) {
	ostream << "x: " << vector[0] << " y: " << vector[1] << " z: " << vector[2] << " w: " << vector[3] << " length: " << vector.Length();
	return ostream;
}

template <class T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator-() {
	Vector<T2, var2> tempVector;
	for (int i = 0; i < var2; ++i)
		tempVector[i] = -m_vector[i];

	return tempVector;
}

template<class T2, int var2>
T2& Vector<T2, var2>::operator[](const int index)
{
	return m_vector[index];
}

template<class T2, int var2>
const T2& Vector<T2, var2>::operator[](const int index) const
{
	return m_vector[index];
}

template <class T2, int var2>
template <class T, int var>
Vector<T2, var2> Vector<T2, var2>::operator*(const Matrix<T, var>& matrix)
{
	static_assert(var == var2, "Size error");
	Vector<T2, var2> tempVector;
	for(int i = 0; i < var2; ++i)
		for(int j = 0; j < var; ++j)
		{
			tempVector[i] += m_vector[j] * matrix[j][i];
		}
	return tempVector;
}

template <typename T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator*(Vector<T2, var2> _vector)
{
    Vector<T2, var2> tempVector;
    for(int i = 0; i < 3; ++i)
        tempVector[i] = m_vector[i] * _vector[i];

    return tempVector;
}

template <typename T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator*=(Vector<T2, var2> _vector)
{
    Vector<T2, var2> tempVector;
    for(int i = 0; i < 3; ++i)
        tempVector[i] = m_vector[i] * _vector[i];

    return tempVector;
}

template <typename T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator-(Vector<T2, var2> _vector)
{
    Vector<T2, var2> temp_Vector(1);

    temp_Vector[0] = m_vector[0] - _vector[0];
    temp_Vector[1] = m_vector[1] - _vector[1];
    temp_Vector[2] = m_vector[2] - _vector[2];

    return temp_Vector;
}

template <typename T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator+(Vector<T2, var2> _vector)
{
        Vector<T2, var2> temp_Vector(1);

    temp_Vector[0] = m_vector[0] + _vector[0];
    temp_Vector[1] = m_vector[1] + _vector[1];
    temp_Vector[2] = m_vector[2] + _vector[2];

    return temp_Vector;
}

template <typename T2, int var2>
void Vector<T2, var2>::operator-=(Vector<T2, var2> _vector)
{
    m_vector[0] = m_vector[0] - _vector[0];
    m_vector[1] = m_vector[1] - _vector[1];
    m_vector[2] = m_vector[2] - _vector[2];
}

template <typename T2, int var2>
void Vector<T2, var2>::operator+=(Vector<T2, var2> _vector)
{
    m_vector[0] = m_vector[0] + _vector[0];
    m_vector[1] = m_vector[1] + _vector[1];
    m_vector[2] = m_vector[2] + _vector[2];
}

template <typename T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator*(T2 _scalar)
{
    Vector<T2, var2> temp_vec(1.0f);
    
    for(int i = 0; i < var2; ++i)
        temp_vec[i] = m_vector[i] * _scalar;

    return temp_vec;
}

template <class T, class T2,int var, int var2>
Matrix<T, var> LookAt(Matrix<T, var> matrix, Vector<T2, var2> vector)
{
	Matrix<T, var> tempMatrix(1.0f);
	tempMatrix = matrix;

	unsigned const int variable = 3;
	for(int i = 0; i < var2; ++i)
		tempMatrix[i][variable] = -vector[i];

	return tempMatrix;
}

template <class T, class T2, int var, int var2>
Matrix<T, var> Translate(Matrix<T, var> matrix, Vector<T2, var2> vector)
{
	Matrix<T, var> tempMatrix(1.0f);
	tempMatrix = matrix;

	// for(int i = 0; i < var; ++i)
	// 	tempMatrix[i][var-1] += vector[i];

	for(int i = 0; i < var; ++i)
		tempMatrix[var-1][i] += vector[i];

	return tempMatrix;
}

template <class T, class T2, int var, int var2>
Matrix<T, var> Scale(Matrix<T, var> matrix, Vector<T2, var2> vector)
{
	Matrix<T, var> tempMatrix;
	tempMatrix = matrix;

	for(int i = 0; i < var; ++i)
		for(int j = 0; j < var; ++j)
		{
			tempMatrix[j][i] *= vector[i];
		}
	return tempMatrix;
}

template <class T, int var>
Matrix<T, var> RotateZ(Matrix<T, var> matrix, float angle)
{
	Matrix<T, var> tempMatrix(1.0f);
	tempMatrix[0][0] = std::cos(angle*PI/180);
	tempMatrix[0][1] = -std::sin(angle*PI/180);
	tempMatrix[1][0] = std::sin(angle*PI/180);
	tempMatrix[1][1] = std::cos(angle*PI/180);

	Matrix<T, var> tempMatrix2;
	tempMatrix2 = matrix * tempMatrix;

	return tempMatrix2;
}

// template <typename T, int size>
// T Deteminant(Matrix<T, size> _matrix)
// {
//     if(size <= 1)
//         return;
//     int iDeterminant = 0;
//     int iSign = 1;

//     Matrix<T, (size - 1)> _temp_Matrix;
    
//     for(int i = 0; i < size; ++i)
//     {
//         if(i % 2 == 1)
//             iSign = -1;
//         else
//             iSign = 1;

//         if(size == 2)
//             return (_matrix[0][i] *iSign) * (_matrix[0][0] * _matrix[1][1] - _matrix[0][1] * _matrix[1][0]);

//         for(int j = 0; j < (size - 1); ++j)
//         {
//             if(j == i)
//                 continue;
//             for(int x = 0; x < (size - 1); ++x)
//             {
//                 if(x == i)
//                     continue;

//                 _temp_Matrix = _matrix[j][x];
//             }
//         }

//         iDeterminant = (_matrix[0][i] * iSign) * iDeterminant(_temp_Matrix);
//     }

//     return iDeterminant;
// }

// template <typename T>
// Vector<T, 3> Cross(Vector<T, 3> _vector1, Vector<T, 3> _vector2)
// {
//     return Vector<T, 3>(_vector1[1] * _vector2[2] - _vector1[2] * _vector2[1],
//                            _vector1[2] * _vector2[0] - _vector1[0] * _vector2[2],
//                            _vector1[0] * _vector2[1] - _vector1[1] * _vector2[0]);
// }

template <typename T>
Vector<T, 3> Cross(Vector<T, 3> _vector1, Vector<T, 3> _vector2)
{
    return Vector<T, 3>(_vector1[1] * _vector2[2] - _vector1[2] * _vector2[1],
                           _vector1[2] * _vector2[0] - _vector1[0] * _vector2[2],
                           _vector1[0] * _vector2[1] - _vector1[1] * _vector2[0]);
}

template <typename T>
T Dot(Vector<T, 3> _vector1, Vector<T, 3> _vector2)
{
    return (_vector1[0] * _vector2[0] + _vector1[1] * _vector2[1] + _vector1[2] * _vector2[2]);
}

template <typename T>
T VectorLength(Vector<T, 3> _vector1, Vector<T, 3> _vector2)
{
    T _x_axis = _vector2[0] - _vector1[0];
    T _y_axis = _vector2[1] - _vector1[1];
    T _z_axis = _vector2[2] - _vector1[2];
    return std::sqrt(_x_axis * _x_axis + _y_axis * _y_axis + _z_axis * _z_axis);
}

template <typename T>
T VecLength(Vector<T, 3> vector)
{
    return std::sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
}

template <typename T>
Vector<T, 3> Normalize(Vector<T, 3> _vector)
{
	if (_vector[0] == 0 && _vector[1] == 0 && _vector[2] == 0)
		return vec3 { 0.0f, 0.0f, 0.0f }; 
	
    float range = std::sqrt(_vector[0]*_vector[0] + _vector[1]*_vector[1] + _vector[2]*_vector[2]);
	for(int l = 0; l < 3; ++l)
		_vector[l] = _vector[l]/range;

    return _vector;
}

// template <typename T>
// Matrix<T, 4> LookAtRH(Vector<T, 3> _eye, Vector<T, 3> _target, Vector<T, 3> _up)
// {
//     Vector<T, 3> z_axis = Normalize(_eye - _target);
//     Vector<T, 3> x_axis = Normalize(Cross(_up, z_axis));
//     Vector<T, 3> y_axis = Cross(z_axis, x_axis);
    
//     Matrix<T, 4> tOrientation(Vector<T, 4>(x_axis[0], y_axis[0], z_axis[0], 0),
//                               Vector<T, 4>(x_axis[1], y_axis[1], z_axis[1], 0),
//                               Vector<T, 4>(x_axis[2], y_axis[2], z_axis[2], 0),
//                               Vector<T, 4>(0, 0, 0, 1));

//     Matrix<T, 4> tTranslation(Vector<T, 4>(1, 0, 0 ,0),
//                               Vector<T, 4>(0, 1, 0, 0),
//                               Vector<T, 4>(0, 0, 1, 0),
//                               Vector<T, 4>(-_eye[0], -_eye[1], -_eye[2], 1));
    
//     return (tOrientation * tTranslation);

//     // Matrix<T, 4> tView(Vector<T, 4>(x_axis[0], y_axis[0], z_axis[0], 0),
//     //                    Vector<T, 4>(x_axis[1], y_axis[1], z_axis[1], 0),
//     //                    Vector<T, 4>(x_axis[2], y_axis[2], z_axis[2], 0),
//     //                    Vector<T, 4>(-Dot(x_axis, _eye), -Dot(y_axis, _eye), -Dot(z_axis, _eye), 1));

//     // return tView;
// }

template <typename T>
Matrix<T, 4> GLVM_perspectiveRH_ZO(T fov, T aspect, T near_plane, T far_plane) {
	T const tanHalfFov = std::tan(fov / static_cast<T>(2));

	mat4 Result(static_cast<T>(0));

	Result[0][0] = static_cast<T>(1) / (aspect * tanHalfFov);
	Result[1][1] = static_cast<T>(1) / (tanHalfFov);
	Result[2][2] = far_plane / (near_plane - far_plane);
	Result[2][3] = - static_cast<T>(1);
	Result[3][2] = -(far_plane * near_plane) / (far_plane - near_plane);
	return Result;
}

template <typename T>
Matrix<T, 4> GLVM_perspectiveRH_NO(T fov, T aspect, T near_plane, T far_plane) {
	T const tanHalfFov = std::tan(fov / static_cast<T>(2));

	mat4 Result(static_cast<T>(0));

	Result[0][0] = static_cast<T>(1) / (aspect * tanHalfFov);
	Result[1][1] = static_cast<T>(1) / (tanHalfFov);
	Result[2][2] = - (far_plane - near_plane) / (far_plane - near_plane);
	Result[2][3] = - static_cast<T>(1);
	Result[3][2] = -(static_cast<T>(2) * far_plane * near_plane) / (far_plane - near_plane);
	return Result;
}

template <typename T>
Matrix<T, 4> Perspective(T fov, T aspect, T near_plane, T far_plane) {
#ifdef GLVM_OPENGL_RENDER_BIT
	return GLVM_perspectiveRH_ZO<T>(fov, aspect, near, far);
#else
	return GLVM_perspectiveRH_NO<T>(fov, aspect, near_plane, far_plane);
#endif
}

template <typename T>
Matrix<T, 4> lookAtRH(Vector<T, 3> _eye, Vector<T, 3> _center, Vector<T, 3> _up)
{
    Vector<T, 3> f = (Normalize(_center - _eye));
    Vector<T, 3> s = (Normalize(Cross(f, _up)));
    Vector<T, 3> u = (Cross(s, f));

    Matrix<T, 4> Result(1.0f);
    Result[0][0] = s[0];
    Result[1][0] = s[1];
    Result[2][0] = s[2];
    Result[0][1] = u[0];
    Result[1][1] = u[1];
    Result[2][1] = u[2];
    Result[0][2] = -f[0];
    Result[1][2] = -f[1];
    Result[2][2] = -f[2];
    Result[3][0] = -Dot(s, _eye);
    Result[3][1] = -Dot(u, _eye);
    Result[3][2] = Dot(f, _eye);
//    Result.SelfTensorTranspose();

    return Result;
}

template<typename T>
Matrix<T, 4> lookAtLH(Vector<T, 3> _eye, Vector<T, 3> _center, Vector<T, 3> _up)
{
	Vector<T, 3> f(Normalize(_center - _eye));
	Vector<T, 3> s(Normalize(Cross(_up, f)));
	Vector<T, 3> u(Cross(f, s));

	Matrix<T, 4> Result(1);
	Result[0][0] = s[0];
	Result[1][0] = s[1];
	Result[2][0] = s[2];
	Result[0][1] = u[0];
	Result[1][1] = u[1];
	Result[2][1] = u[2];
	Result[0][2] = f[0];
	Result[1][2] = f[1];
	Result[2][2] = f[2];
	Result[3][0] = -Dot(s, _eye);
	Result[3][1] = -Dot(u, _eye);
	Result[3][2] = -Dot(f, _eye);
	return Result;
}

template <typename T>
Matrix<T, 4> LookAtMain(Vector<T, 3> _eye, Vector<T, 3> _center, Vector<T, 3> _up)
{
#ifdef GLVM_OPENGL_RENDER_BIT
	return lookAtLH<T>(_eye, _center, _up);
#else
	return lookAtRH<T>(_eye, _center, _up);
#endif
}

template <typename T>
Matrix<T, 4> FPSview(Vector<T, 3> _eye, Vector<T, 3> _center, Vector<T, 3> _up)
{
//    _center.m_vector[1] = _eye.m_vector[1];
    
    Vector<T, 3> f(Normalize(_center - _eye));
    Vector<T, 3> s(Normalize(Cross(f, _up)));
    Vector<T, 3> u(Cross(s, f));

    Matrix<T, 4> Result(1.0f);
    Result[0][0] = s[0];
    Result[1][0] = s[1];
    Result[2][0] = s[2];
    Result[0][1] = u[0];
    Result[1][1] = u[1];
    Result[2][1] = u[2];
    Result[0][2] = -f[0];
    Result[1][2] = -f[1];
    Result[2][2] = -f[2];
    Result[3][0] = -Dot(s, _eye);
    Result[3][1] = -Dot(u, _eye);
    Result[3][2] = Dot(f, _eye);
//    Result.SelfTensorTranspose();

    return Result;
}

template <typename T3>
T3 Radians(T3 _angle)
{
    _angle *= PI / static_cast<T3>(180);

    return _angle;
}

template <typename T>
Matrix<T, 4> FPS_View_RH(Vector<T, 3> _eye, float _pitch, float _yaw)
{
    _pitch *= PI / 180;
    _yaw *= PI / 180;

    float fCos_Pitch = std::cos(_pitch);
    float fSin_Pitch = std::sin(_pitch);
    float fCos_Yaw = std::cos(_yaw);
    float fSin_Yaw = std::sin(_yaw);

    Vector<T, 3> x_axis(fCos_Yaw, 0, -fSin_Yaw);
    Vector<T, 3> y_axis(fSin_Yaw * fSin_Pitch, fCos_Pitch, fCos_Yaw * fSin_Pitch);
    Vector<T, 3> z_axis(fSin_Yaw * fCos_Pitch, -fSin_Pitch, fCos_Pitch * fCos_Yaw);

    Matrix<T, 4> tView(Vector<T, 4>(x_axis[0], y_axis[0], z_axis[0], 0),
                       Vector<T, 4>(x_axis[1], y_axis[1], z_axis[1], 0),
                       Vector<T, 4>(x_axis[2], y_axis[2], z_axis[2], 0),
                       Vector<T, 4>(-Dot(x_axis, _eye), -Dot(y_axis, _eye), -Dot(z_axis, _eye), 1));

    return tView;
}

template <class T, int var, int vec_size>
Matrix<T, var> Rotate(Vector<T, vec_size> vector, float angle)
{
    vector = (Normalize(vector));

//	angle *= PI / 180;
	Matrix<T, var> tempMatrix(1.0f);

    /// Transposed rotate matrix.
    
	tempMatrix[0][0] = std::cos(angle) + (vector[0] * vector[0]) * (static_cast<T>(1) - std::cos(angle));
	tempMatrix[1][0] = vector[0] * vector[1] * (static_cast<T>(1) - std::cos(angle)) - vector[2] * std::sin(angle);
	tempMatrix[2][0] = vector[0] * vector[2] * (static_cast<T>(1) - std::cos(angle)) + vector[1] * std::sin(angle);
	tempMatrix[3][0] = static_cast<T>(0);
	tempMatrix[0][1] = vector[1] * vector[0] * (static_cast<T>(1) - std::cos(angle)) + vector[2] * std::sin(angle);
	tempMatrix[1][1] = std::cos(angle) + (vector[1] * vector[1]) * (static_cast<T>(1) - std::cos(angle));
	tempMatrix[2][1] = vector[1] * vector[2] * (static_cast<T>(1) - std::cos(angle)) - vector[0] * std::sin(angle);
	tempMatrix[3][1] = static_cast<T>(0);
	tempMatrix[0][2] = vector[2] * vector[0] * (static_cast<T>(1) - std::cos(angle)) - vector[1] * std::sin(angle);
	tempMatrix[1][2] = vector[2] * vector[1] * (static_cast<T>(1) - std::cos(angle)) + vector[0] * std::sin(angle);
	tempMatrix[2][2] = std::cos(angle) + (vector[2] * vector[2]) * (static_cast<T>(1) - std::cos(angle));
	tempMatrix[3][2] = static_cast<T>(0);
	tempMatrix[0][3] = static_cast<T>(0);
	tempMatrix[1][3] = static_cast<T>(0);
	tempMatrix[2][3] = static_cast<T>(0);
	tempMatrix[3][3] = static_cast<T>(1);

    /// Normal rotate matrix.
    
	// tempMatrix[0][0] = std::cos(angle) + (vector[0] * vector[0]) * (static_cast<T>(1) - std::cos(angle));
	// tempMatrix[0][1] = vector[0] * vector[1] * (static_cast<T>(1) - std::cos(angle)) - vector[2] * std::sin(angle);
	// tempMatrix[0][2] = vector[0] * vector[2] * (static_cast<T>(1) - std::cos(angle)) + vector[1] * std::sin(angle);
	// tempMatrix[0][3] = static_cast<T>(0);
	// tempMatrix[1][0] = vector[1] * vector[0] * (static_cast<T>(1) - std::cos(angle)) + vector[2] * std::sin(angle);
	// tempMatrix[1][1] = std::cos(angle) + (vector[1] * vector[1]) * (static_cast<T>(1) - std::cos(angle));
	// tempMatrix[1][2] = vector[1] * vector[2] * (static_cast<T>(1) - std::cos(angle)) - vector[0] * std::sin(angle);
	// tempMatrix[1][3] = static_cast<T>(0);
	// tempMatrix[2][0] = vector[2] * vector[0] * (static_cast<T>(1) - std::cos(angle)) - vector[1] * std::sin(angle);
	// tempMatrix[2][1] = vector[2] * vector[1] * (static_cast<T>(1) - std::cos(angle)) + vector[0] * std::sin(angle);
	// tempMatrix[2][2] = std::cos(angle) + (vector[2] * vector[2]) * (static_cast<T>(1) - std::cos(angle));
	// tempMatrix[2][3] = static_cast<T>(0);
	// tempMatrix[3][0] = static_cast<T>(0);
	// tempMatrix[3][1] = static_cast<T>(0);
	// tempMatrix[3][2] = static_cast<T>(0);
	// tempMatrix[3][3] = static_cast<T>(1);
    
    // tempMatrix[0][0] = std::cos(angle) + (vector[0] * vector[0]) * (1 - std::cos(angle));
	// tempMatrix[0][1] = vector[0] * vector[1] * (1 - std::cos(angle)) - vector[2] * std::sin(angle);
	// tempMatrix[0][2] = vector[0] * vector[2] * (1 - std::cos(angle)) + vector[1] * std::sin(angle);
	// tempMatrix[0][3] = 0;
	// tempMatrix[1][0] = vector[1] * vector[0] * (1 - std::cos(angle)) + vector[2] * std::sin(angle);
	// tempMatrix[1][1] = std::cos(angle) + (vector[1] * vector[1]) * (1 - std::cos(angle));
	// tempMatrix[1][2] = vector[1] * vector[2] * (1 - std::cos(angle)) - vector[0] * std::sin(angle);
	// tempMatrix[1][3] = 0;
	// tempMatrix[2][0] = vector[2] * vector[0] * (1 - std::cos(angle)) - vector[1] * std::sin(angle);
	// tempMatrix[2][1] = vector[2] * vector[1] * (1 - std::cos(angle)) + vector[0] * std::sin(angle);
	// tempMatrix[2][2] = std::cos(angle) + (vector[2] * vector[2]) * (1 - std::cos(angle));
	// tempMatrix[2][3] = 0;
	// tempMatrix[3][0] = 0;
	// tempMatrix[3][1] = 0;
	// tempMatrix[3][2] = 0;
	// tempMatrix[3][3] = 1;

    // Matrix<float, 4> model_mat(1.0f);
    // model_mat = matrix * tempMatrix;
    
	return tempMatrix;
}

template <class T, int var>
Matrix<T, var> Ortho(float w, float h, float zn, float zf)
{
	Matrix<T, var> tempMatrix(1.0f);
	tempMatrix[0][0] = 2/w;
	tempMatrix[1][1] = 2/h;
	tempMatrix[2][2] = 1/(zf-zn);

	return tempMatrix;
}

template <class T>
Matrix<T, 4> orthoRH_ZO(T left, T right, T bottom, T top, T near_plane, T far_plane) {
	Matrix<float, 4> tempMatrix(1);
	tempMatrix[0][0] = static_cast<T>(2) / (right - left);
	tempMatrix[1][1] = static_cast<T>(2) / (top - bottom);
	tempMatrix[2][2] = - static_cast<T>(1) / (far_plane - near_plane);
	tempMatrix[3][0] = - (right + left) / (right - left);
	tempMatrix[3][1] = - (top + bottom) / (top - bottom);
	tempMatrix[3][2] = - near_plane / (far_plane - near_plane);

	return tempMatrix;
}

template <class T>
Matrix<T, 4> orthoLH_NO(T left, T right, T bottom, T top, T near_plane, T far_plane) {
	Matrix<float, 4> tempMatrix(1);
	tempMatrix[0][0] = static_cast<T>(2) / (right - left);
	tempMatrix[1][1] = static_cast<T>(2) / (top - bottom);
	tempMatrix[2][2] = - static_cast<T>(2) / (far_plane - near_plane);
	tempMatrix[3][0] = - (right + left) / (right - left);
	tempMatrix[3][1] = - (top + bottom) / (top - bottom);
	tempMatrix[3][2] = - (far_plane + near_plane) / (far_plane - near_plane);

	return tempMatrix;
}

template <class T>
Matrix<T, 4> ortho(T left, T right, T bottom, T top, T near_plane, T far_plane) {
#ifdef GLVM_OPENGL_RENDER_BIT
	return orthoLH_NO<T>(left, right, bottom, top, near, far);
#else
	return orthoRH_ZO<T>(left, right, bottom, top, near_plane, far_plane);
#endif
}

template <class T, int var>
Matrix<T, var> perspectiveRH_ZO(T fov, T aspect, T near_plane, T far_plane) {
	float tanHalfFov = std::tan((fov / 2) * (PI / 360));
	
	Matrix<float, var> tempMatrix(static_cast<T>(0));
	tempMatrix[0][0] = static_cast<T>(1) / (aspect * tanHalfFov);
	tempMatrix[1][1] = static_cast<T>(1) / tanHalfFov;
	tempMatrix[2][2] = far_plane / (near_plane - far_plane);
	tempMatrix[2][3] = static_cast<T>(1);
	tempMatrix[3][2] = -(far_plane * near_plane) / (far_plane - near_plane);

	return tempMatrix;
}

template <class T, int var>
Matrix<T, var> perspectiveLH_NO(T fov, T aspect, T near_plane, T far_plane) {
	float tanHalfFov = std::tan((fov / 2) * (PI / 360));
	
	Matrix<float, var> tempMatrix(static_cast<T>(0));
	tempMatrix[0][0] = static_cast<T>(1) / (aspect * tanHalfFov);
	tempMatrix[1][1] = static_cast<T>(1) / tanHalfFov;
	tempMatrix[2][2] = - (far_plane + near_plane) / (far_plane - near_plane);
	tempMatrix[2][3] = - static_cast<T>(1);
	tempMatrix[3][2] = - (static_cast<T>(2) * far_plane * near_plane) / (far_plane - near_plane);

	return tempMatrix;
}

template <class T, int var>
Matrix<T, var> Perspective(T fov, T aspect, T near_plane, T far_plane) {
#ifdef GLVM_OPENGL_RENDER_BIT
	return perspectiveLH_NO(fov, aspect, near, far);
#else
	return perspectiveRH_ZO(fov, aspect, near_plane, far_plane);
#endif
	// float S = std::tan((fov/2)*(PI/360));
	// Matrix<float, var> tempMatrix;
	// tempMatrix[0][0] = 1/((1280.0f/1280.0f)*S);
	// tempMatrix[1][1] = 1/S;
	// tempMatrix[2][2] = -((f+n)/(f-n));
	// tempMatrix[2][3] = -((2*f*n)/(f-n));
	// tempMatrix[3][2] = -1;
}

constexpr float Max(float var1, float var2) {
	return var1 > var2 ? var1 : var2;
}

constexpr float Min(float var1, float var2) {
	return var1 < var2 ? var1 : var2;
}

struct Quaternion
{
	float w, x, y, z;
};

inline std::ostream& operator<<(std::ostream& ostream, const Quaternion& quaternion) {
	ostream << "w: " << quaternion.w << " x: " << quaternion.x << " y: " << quaternion.y << " z: " << quaternion.z;
	
	return ostream;
}

inline Quaternion linkedQuaternionValue(Quaternion quaternion) {
	quaternion.x = -quaternion.x;
	quaternion.y = -quaternion.y;
	quaternion.z = -quaternion.z;
	
	return quaternion;
}

inline Quaternion multiplyQuaternion(Quaternion a, Quaternion b) {
	Quaternion result;
	result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
	result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;

	return result;
}

inline float normQuaternion(Quaternion quaternion) {
	return sqrt(quaternion.w * quaternion.w + quaternion.x * quaternion.x +
				quaternion.y * quaternion.y + quaternion.z * quaternion.z);
}

inline Quaternion normalizeQuaternion(Quaternion quaternion) {
	float norm = normQuaternion(quaternion);
	
	quaternion.w /= norm;
	quaternion.x /= norm;
	quaternion.y /= norm;
	quaternion.z /= norm;

	return quaternion;
}

inline Quaternion inverseQuaternion(Quaternion quaternion) {
	Quaternion linkedValue = linkedQuaternionValue(quaternion);
	float norm = normQuaternion(quaternion);

	quaternion.w = linkedValue.w / norm;
	quaternion.x = linkedValue.x / norm;
	quaternion.y = linkedValue.y / norm;
	quaternion.z = linkedValue.z / norm;
	
	return quaternion;
}

inline Quaternion eulerToQuaternion(float roll, float pitch, float yaw) {
	float cr = cos(roll * 0.5);
    float sr = sin(roll * 0.5);
    float cp = cos(pitch * 0.5);
    float sp = sin(pitch * 0.5);
    float cy = cos(yaw * 0.5);
    float sy = sin(yaw * 0.5);

    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

	return q;
}

template <class T, int var>
Matrix<T, var> rotateQuaternion(Quaternion quaternion) {
	mat4 result(0.0f);

	quaternion = normalizeQuaternion(quaternion);

	result[0][0] = 1 - 2 * (quaternion.y * quaternion.y + quaternion.z * quaternion.z);
	result[0][1] = 2 * (quaternion.x * quaternion.y - quaternion.z * quaternion.w);
	result[0][2] = 2 * (quaternion.x * quaternion.z + quaternion.y * quaternion.w);

	result[1][0] = 2 * (quaternion.x * quaternion.y + quaternion.z * quaternion.w);
	result[1][1] = 1 - 2 * (quaternion.x * quaternion.x + quaternion.z * quaternion.z);
	result[1][2] = 2 * (quaternion.y * quaternion.z - quaternion.x * quaternion.w);

	result[2][0] = 2 * (quaternion.x * quaternion.z - quaternion.y * quaternion.w);
	result[2][1] = 2 * (quaternion.y * quaternion.z + quaternion.x * quaternion.w);
	result[2][2] = 1 - 2 * (quaternion.x * quaternion.x + quaternion.y * quaternion.y);
	
	result[0][0] = 2 * (quaternion.w * quaternion.w + quaternion.x * quaternion.x) - 1;
	result[0][1] = 2 * (quaternion.x * quaternion.y - quaternion.w * quaternion.z);
	result[0][2] = 2 * (quaternion.x * quaternion.z + quaternion.w * quaternion.y);

	result[3][3] = 1.0f;

	return result;
}

#endif
