//
//  Instruction.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/12/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import Foundation

/**
 Struct which can store an instruction
 */
struct Instruction {
    var distance: Float
    var angle: Float = 0
    var waypointNumber: Int
}
