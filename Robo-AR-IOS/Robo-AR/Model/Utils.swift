//
//  Utils.swift
//  Robo-AR
//
//  Created by Anmol Parande on 12/10/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import Foundation
import RealityKit

extension SIMD3 where Scalar == Float {
    var lengthHorizontal: Float {
        return sqrtf(x * x  + z * z)
    }
    
    func horizontalAngle(to other:SIMD3<Float>) -> Float {
        var angle = acos(dot(self, other) / (length(other) * length(self))) * 180 / .pi
        if cross(self, other).y  < 0 {
            angle = -1 * angle
        }
        return angle
    }
}
