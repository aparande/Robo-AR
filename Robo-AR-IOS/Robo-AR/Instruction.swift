//
//  Instruction.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/12/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import Foundation

struct Instruction {
    var distance: Float
    var angle: Float = 0
    var completed = false
    var waypointNumber: Int
}
