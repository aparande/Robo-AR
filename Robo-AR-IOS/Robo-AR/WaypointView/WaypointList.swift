//
//  WaypointList.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/8/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import Foundation

// Linked List of Waypoints
class WaypointList {
    var head: Waypoint?
    var tail: Waypoint?
    
    var isEmpty: Bool {
        get {
            return head == nil
        }
    }
    
    func insert(_ point: Waypoint) {
        if isEmpty {
            head = point
            tail = point
        } else {
            tail?.next = point
            tail = point
        }
    }
}
