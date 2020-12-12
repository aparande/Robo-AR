//
//  TrackedObject.swift
//  Robo-AR
//
//  Created by Anmol Parande on 12/10/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import Foundation

protocol TrackedObject {
    func distanceTo(_ other: ARPoint) -> Float
    func angleTo(_ other: ARPoint) -> Float
}
