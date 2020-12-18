//
//  TrackedObject.swift
//  Robo-AR
//
//  Created by Anmol Parande on 12/10/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import Foundation

/**
 Interface which defines an object that can be localized in space (and hence tracked)
*/
protocol TrackedObject {
    /**
     Compute the distance to an AR Point
     */
    func distanceTo(_ other: ARPoint) -> Float
    
    /**
     Compute the angle that needs to be turned to orient towards an AR Point
     */
    func angleTo(_ other: ARPoint) -> Float
}
