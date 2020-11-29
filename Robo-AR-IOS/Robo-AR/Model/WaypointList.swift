//
//  WaypointList.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/8/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import Foundation

// Linked List of Waypoints
class WaypointList: Sequence {
    var head: Waypoint?
    var tail: Waypoint?
    var count = 0
    
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
        count += 1
    }
    
    func makeIterator() -> WaypointIterator {
        return WaypointIterator(withList: self)
    }
}

struct WaypointIterator: IteratorProtocol {
    var current: Waypoint?

    init(withList list: WaypointList) {
        current = list.head
    }
    
    mutating func next() -> Waypoint? {
        guard let curr = current else { return nil }
        current = curr.next
        return curr
    }
}
