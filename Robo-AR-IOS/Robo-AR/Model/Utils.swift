//
//  Utils.swift
//  Robo-AR
//
//  Created by Anmol Parande on 12/10/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import Foundation
import RealityKit

/**
 Extend the built-in SIMD3 vector to support some useful operations
 */
extension SIMD3 where Scalar == Float {
    /**
     Compute the magnitude of the vector projected on the xz-plane
     */
    var lengthHorizontal: Float {
        return sqrtf(x * x  + z * z)
    }
    
    /**
     Compute the angle between two SIMD3 vectors in the xz-plane
     */
    func horizontalAngle(to other:SIMD3<Float>) -> Float {
        // Get the angle using the dot product
        var angle = acos(dot(self, other) / (length(other) * length(self))) * 180 / .pi
        
        // Since acos returns values between 0 and 180, use the cross product to get negative angles
        if cross(self, other).y  < 0 {
            angle = -1 * angle
        }
        return angle
    }
}
