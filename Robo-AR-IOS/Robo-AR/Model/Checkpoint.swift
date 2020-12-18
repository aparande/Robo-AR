//
//  Checkpoint.swift
//  Robo-AR
//
//  Created by Anmol Parande on 12/10/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import Foundation

/**
 Abstraction which "checkpoints" where the robot is
 */
struct Checkpoint: TrackedObject {
    // Reference either to the robot or a waypoint
    var reference: ARPoint
    
    // How is the robot oriented relative to its original orientation
    var orientation: Float
    
    // Compute the distance to another AR Point
    func distanceTo(_ other: ARPoint) -> Float {
        return reference.distanceTo(other)
    }
    
    // Compute the angle to another AR Point
    func angleTo(_ other: ARPoint) -> Float {
        print("If robot faces straight, turn \(reference.angleTo(other)), but robot is facing \(orientation)")
        
        // Subtract out the orientation because, for example, if the robot is turned 45˚ and needs to turn to 90˚, it only needs to turn 90˚ - 45˚
        return reference.angleTo(other) - orientation
    }
}
