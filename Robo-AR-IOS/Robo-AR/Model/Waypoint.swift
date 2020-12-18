import UIKit
import RealityKit

/**
 Waypoint Class. Is an instance of a more general AR Point
 */
class Waypoint: ARPoint {
    // The waypoint number
    let number: Int
    
    // A link to the next waypoint (since these are also nodes in a Waypoint list)
    var next: Waypoint? {
        didSet {
            // Construct the line from this waypoint to the next
            guard let next = next else { return }
            next.prev = self
            print("Distance from \(number) to \(next.number) is \(distanceTo(next))")
        }
    }
    
    // A link to the previous waypoint
    var prev: Waypoint?
    
    // Is this waypoint the last in the waypoint list?
    var isTerminal: Bool {
        get {
            return next == nil
        }
    }
    
    required init(color: UIColor, number: Int) {
        self.number = number
        super.init(color: color, text: "\(self.number)")
    }
    
    required init() {
        fatalError("init() has not been implemented")
    }
}
