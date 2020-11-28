import UIKit
import RealityKit

class Waypoint: ARPoint {
    let number: Int
    var next: Waypoint? {
        didSet {
            // Construct the line from this waypoint to the next
            guard let next = next else { return }
            next.prev = self
            print("Distance from \(number) to \(next.number) is \(distanceTo(next))")
        }
    }
    
    var prev: Waypoint?
    
    var isTerminal: Bool {
        get {
            return prev == nil
        }
    }
    
    required init(color: UIColor, number: Int) {
        self.number = number
        super.init(color: color, text: "\(self.number)")
    }
    
    required init() {
        fatalError("init() has not been implemented")
    }
        
    override func angleTo(_ other: ARPoint) -> Float {
        return self.parent!.position.horizontalAngle(to: other.parent!.position)
    }
}
