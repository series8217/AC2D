//** From https://cs.stanford.edu/~acoates/quaternion.h

#pragma once

#include <iostream>
#include <math.h>
#include "tmatrix.h"

#define M_PI 3.14159265358979323846


class Quaternion {
    double mData[4];

public:

    Quaternion() {
        mData[0] = mData[1] = mData[2] = 0;
        mData[3] = 1;
    }

    Quaternion(const TVector3& v, double W) {
        mData[0] = v.element(0, 0);
        mData[1] = v.element(1, 0);
        mData[2] = v.element(2, 0);
        mData[3] = W;
    }

    Quaternion(const TVector4& v) {
        mData[0] = v.element(0, 0);
        mData[1] = v.element(1, 0);
        mData[2] = v.element(2, 0);
        mData[3] = v.element(3, 0);
    }

    Quaternion(const double* array) {
        if (!array) {
            throw std::invalid_argument("array is NULL");
        }
        for (uint32_t i = 0; i < 4; i++) {
            mData[i] = array[i];
        }
    }

    Quaternion(double X, double Y, double Z, double W) {
        mData[0] = X;
        mData[1] = Y;
        mData[2] = Z;
        mData[3] = W;
    }

    double X() const { return mData[0]; }
    double Y() const { return mData[1]; }
    double Z() const { return mData[2]; }
    double W() const { return real(); }

    TVector3 complex() const { return TVector3(mData); }
    void complex(const TVector3& c) { mData[0] = c[0]; mData[1] = c[1];  mData[2] = c[2]; }

    double real() const { return mData[3]; }
    void real(double r) { mData[3] = r; }

    Quaternion conjugate(void) const {
        return Quaternion(-complex(), real());
    }

    /**
     * @brief Computes the inverse of this quaternion.
     *
     * @note This is a general inverse.  If you know a priori
     * that you're using a unit quaternion (i.e., norm() == 1),
     * it will be significantly faster to use conjugate() instead.
     *
     * @return The quaternion q such that q * (*this) == (*this) * q
     * == [ 0 0 0 1 ]<sup>T</sup>.
     */
    Quaternion inverse(void) const {
        return conjugate() / norm();
    }


    /**
     * @brief Computes the product of this quaternion with the
     * quaternion 'rhs'.
     *
     * @param rhs The right-hand-side of the product operation.
     *
     * @return The quaternion product (*this) X @p rhs.
     */
    Quaternion product(const Quaternion& rhs) const {
        return Quaternion(Y()*rhs.Z() - Z()*rhs.Y() + X()*rhs.W() + W()*rhs.X(),
            Z()*rhs.X() - X()*rhs.Z() + Y()*rhs.W() + W()*rhs.Y(),
            X()*rhs.Y() - Y()*rhs.X() + Z()*rhs.W() + W()*rhs.Z(),
            W()*rhs.W() - X()*rhs.X() - Y()*rhs.Y() - Z()*rhs.Z());
    }

    /**
     * @brief Quaternion product operator.
     *
     * The result is a quaternion such that:
     *
     * result.real() = (*this).real() * rhs.real() -
     * (*this).complex().dot(rhs.complex());
     *
     * and:
     *
     * result.complex() = rhs.complex() * (*this).real
     * + (*this).complex() * rhs.real()
     * - (*this).complex().cross(rhs.complex());
     *
     * @return The quaternion product (*this) X rhs.
     */
    Quaternion operator*(const Quaternion& rhs) const {
        return product(rhs);
    }

    /**
     * @brief Quaternion scalar product operator.
     * @param s A scalar by which to multiply all components
     * of this quaternion.
     * @return The quaternion (*this) * s.
     */
    Quaternion operator*(double s) const {
        return Quaternion(complex()*s, real()*s);
    }

    /**
     * @brief Produces the sum of this quaternion and rhs.
     */
    Quaternion operator+(const Quaternion& rhs) const {
        return Quaternion(X() + rhs.X(), Y() + rhs.Y(), Z() + rhs.Z(), W() + rhs.W());
    }

    /**
     * @brief Produces the difference of this quaternion and rhs.
     */
    Quaternion operator-(const Quaternion& rhs) const {
        return Quaternion(X() - rhs.X(), Y() - rhs.Y(), Z() - rhs.Z(), W() - rhs.W());
    }

    /**
     * @brief Unary negation.
     */
    Quaternion operator-() const {
        return Quaternion(-X(), -Y(), -Z(), -W());
    }

    /**
     * @brief Quaternion scalar division operator.
     * @param s A scalar by which to divide all components
     * of this quaternion.
     * @return The quaternion (*this) / s.
     */
    Quaternion operator/(double s) const {
        if (s == 0) std::clog << "Dividing quaternion by 0." << std::endl;
        return Quaternion(complex() / s, real() / s);
    }

    /**
     * @brief Returns a matrix representation of this
     * quaternion.
     *
     * Specifically this is the matrix such that:
     *
     * this->matrix() * q.vector() = (*this) * q for any quaternion q.
     *
     * Note that this is @e NOT the rotation matrix that may be
     * represented by a unit quaternion.
     */
    TMatrix4 matrix() const {
        double m[16] = {
           W(), -Z(),  Y(), X(),
           Z(),  W(), -X(), Y(),
          -Y(),  X(),  W(), Z(),
          -X(), -Y(), -Z(), W()
        };
        return TMatrix4(m);
    }

    /**
     * @brief Returns a matrix representation of this
     * quaternion for right multiplication.
     *
     * Specifically this is the matrix such that:
     *
     * q.vector().transpose() * this->matrix() = (q *
     * (*this)).vector().transpose() for any quaternion q.
     *
     * Note that this is @e NOT the rotation matrix that may be
     * represented by a unit quaternion.
     */
    TMatrix4 rightMatrix() const {
        double m[16] = {
          +W(), -Z(),  Y(), -X(),
          +Z(),  W(), -X(), -Y(),
          -Y(),  X(),  W(), -Z(),
          +X(),  Y(),  Z(),  W()
        };
        return TMatrix4(m);
    }

    /**
     * @brief Returns this quaternion as a 4-vector.
     *
     * This is simply the vector [X Y Z W]<sup>T</sup>
     */
    TVector4 vector() const { return TVector4(mData); }

    /**
     * @brief Returns the norm ("magnitude") of the quaternion.
     * @return The 2-norm of [ W(), X(), Y(), Z() ]<sup>T</sup>.
     */
    double norm() const {
        return sqrt(mData[0] * mData[0] + mData[1] * mData[1] +
            mData[2] * mData[2] + mData[3] * mData[3]);
    }

    /**
     * @brief Computes the rotation matrix represented by a unit
     * quaternion.
     *
     * @note This does not check that this quaternion is normalized.
     * It formulaically returns the matrix, which will not be a
     * rotation if the quaternion is non-unit.
     */
    TMatrix3 rotationMatrix() const {
        double m[9] = {
          1 - 2 * Y()*Y() - 2 * Z()*Z(), 2 * X()*Y() - 2 * Z()*W(), 2 * X()*Z() + 2 * Y()*W(),
          2 * X()*Y() + 2 * Z()*W(), 1 - 2 * X()*X() - 2 * Z()*Z(), 2 * Y()*Z() - 2 * X()*W(),
          2 * X()*Z() - 2 * Y()*W(), 2 * Y()*Z() + 2 * X()*W(), 1 - 2 * X()*X() - 2 * Y()*Y()
        };
        return TMatrix3(m);
    }

    /**
     * @brief Returns the scaled-axis representation of this
     * quaternion rotation.
     */
    // TODO: implement me
    //TVector3 scaledAxis(void) const {
    //    double W[3];
    //    HeliMath::scaled_axis_from_quaternion(W, mData);
    //    return TVector3(W);
    //}

    /**
     * @brief Sets quaternion to be same as rotation by scaled axis W.
     */
    void scaledAxis(const TVector3& W) {
        double theta = W.norm();
        if (theta > 0.0001) {
            double s = sin(theta / 2.0);
            TVector3 W(W / theta * s);
            mData[0] = W[0];
            mData[1] = W[1];
            mData[2] = W[2];
            mData[3] = cos(theta / 2.0);
        }
        else {
            mData[0] = mData[1] = mData[2] = 0;
            mData[3] = 1.0;
        }
    }

    /**
     * @brief Returns a vector rotated by this quaternion.
     *
     * Functionally equivalent to:  (rotationMatrix() * v)
     * or (q * Quaternion(0, v) * q.inverse()).
     *
     * @warning conjugate() is used instead of inverse() for better
     * performance, when this quaternion must be normalized.
     */
    TVector3 rotatedVector(const TVector3& v) const {
        return (((*this) * Quaternion(v, 0)) * conjugate()).complex();
    }



    /**
     * @brief Computes the quaternion that is equivalent to a given
     * euler angle rotation.
     * @param euler A 3-vector in order:  roll-pitch-yaw.
     */
    void euler(const TVector3& euler) {
        double c1 = cos(euler[2] * 0.5);
        double c2 = cos(euler[1] * 0.5);
        double c3 = cos(euler[0] * 0.5);
        double s1 = sin(euler[2] * 0.5);
        double s2 = sin(euler[1] * 0.5);
        double s3 = sin(euler[0] * 0.5);

        mData[0] = c1 * c2*s3 - s1 * s2*c3;
        mData[1] = c1 * s2*c3 + s1 * c2*s3;
        mData[2] = s1 * c2*c3 - c1 * s2*s3;
        mData[3] = c1 * c2*c3 + s1 * s2*s3;
    }

    /** @brief Returns an equivalent euler angle representation of
     * this quaternion.
     * @return Euler angles in roll-pitch-yaw order.
     */
    TVector3 euler(void) const {
        TVector3 euler;
        const static double PI_OVER_2 = M_PI * 0.5;
        const static double EPSILON = 1e-10;
        double sqw, sqx, sqy, sqz;

        // quick conversion to Euler angles to give tilt to user
        sqw = mData[3] * mData[3];
        sqx = mData[0] * mData[0];
        sqy = mData[1] * mData[1];
        sqz = mData[2] * mData[2];

        euler[1] = asin(2.0 * (mData[3] * mData[1] - mData[0] * mData[2]));
        if (PI_OVER_2 - fabs(euler[1]) > EPSILON) {
            euler[2] = atan2(2.0 * (mData[0] * mData[1] + mData[3] * mData[2]),
                sqx - sqy - sqz + sqw);
            euler[0] = atan2(2.0 * (mData[3] * mData[0] + mData[1] * mData[2]),
                sqw - sqx - sqy + sqz);
        }
        else {
            // compute heading from local 'down' vector
            euler[2] = atan2(2 * mData[1] * mData[2] - 2 * mData[0] * mData[3],
                2 * mData[0] * mData[2] + 2 * mData[1] * mData[3]);
            euler[0] = 0.0;

            // If facing down, reverse yaw
            if (euler[1] < 0)
                euler[2] = M_PI - euler[2];
        }
        return euler;
    }

    /**
     * @brief Computes a special representation that decouples the Z
     * rotation.
     *
     * The decoupled representation is two rotations, Qxy and Qz,
     * so that Q = Qxy * Qz.
     */
    // TODO implement me
    //void decoupleZ(Quaternion* Qxy, Quaternion* Qz) const {
    //    TVector3 ztt(0, 0, 1);
    //    TVector3 zbt = this->rotatedVector(ztt);
    //    TVector3 axis_xy = ztt.cross(zbt);
    //    double axis_norm = axis_xy.norm();

    //    double axis_theta = acos(HeliMath::saturate(zbt[2], -1, +1));
    //    if (axis_norm > 0.00001) {
    //        axis_xy = axis_xy * (axis_theta / axis_norm); // limit is *1
    //    }

    //    Qxy->scaledAxis(axis_xy);
    //    *Qz = (Qxy->conjugate() * (*this));
    //}

    /**
     * @brief Returns the quaternion slerped between this and q1 by fraction 0 <= t <= 1.
     */
    Quaternion slerp(const Quaternion& q1, double t) {
        return slerp(*this, q1, t);
    }

    /// Returns quaternion that is slerped by fraction 't' between q0 and q1.
    // TODO: implement me
    //static Quaternion slerp(const Quaternion& q0, const Quaternion& q1, double t) {

    //    double omega = acos(HeliMath::saturate(q0.mData[0] * q1.mData[0] +
    //        q0.mData[1] * q1.mData[1] +
    //        q0.mData[2] * q1.mData[2] +
    //        q0.mData[3] * q1.mData[3], -1, 1));
    //    if (fabs(omega) < 1e-10) {
    //        omega = 1e-10;
    //    }
    //    double som = sin(omega);
    //    double st0 = sin((1 - t) * omega) / som;
    //    double st1 = sin(t * omega) / som;

    //    return Quaternion(q0.mData[0] * st0 + q1.mData[0] * st1,
    //        q0.mData[1] * st0 + q1.mData[1] * st1,
    //        q0.mData[2] * st0 + q1.mData[2] * st1,
    //        q0.mData[3] * st0 + q1.mData[3] * st1);
    //}

    /**
     * @brief Returns pointer to the internal array.
     *
     * Array is in order X,Y,Z,W.
     */
    double* row(uint32_t i) { return mData + i; }
    // Const version of the above.
    const double* row(uint32_t i) const { return mData + i; }
};

/**
 * @brief Global operator allowing left-multiply by scalar.
 */
Quaternion operator*(double s, const Quaternion& q);

