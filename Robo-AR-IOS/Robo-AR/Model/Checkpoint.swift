//
//  Checkpoint.swift
//  Robo-AR
//
//  Created by Anmol Parande on 12/10/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import Foundation

struct Checkpoint: TrackedObject {
    var reference: ARPoint
    var orientation: Float // How is the robot oriented relative to the straight ahead of the origin
    
    func distanceTo(_ other: ARPoint) -> Float {
        return reference.distanceTo(other)
    }
    
    func angleTo(_ other: ARPoint) -> Float {
        print("If robot faces straight, turn \(reference.angleTo(other)), but robot is facing \(orientation)")
        return reference.angleTo(other) - orientation
    }
}
