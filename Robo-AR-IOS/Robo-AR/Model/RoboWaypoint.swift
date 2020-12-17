
import UIKit
import RealityKit

/**
 Waypoint for the robot location.
 Subclasses AR Point and implements the TrackedObject interface
 */
class RoboWaypoint: ARPoint, TrackedObject {
    required init(color: UIColor) {
        super.init(color: color, text: "Robot")
        
        self.forwardAngle = SIMD3<Float>(0.0, 0.0, 1.0)
        addSphere()
    }
    
    required init() {
        fatalError("init() has not been implemented")
    }
            
    // Add a sphere to the robot geometry to represent where forward is
    private func addSphere() {
        let mesh2 = MeshResource.generateSphere(radius: 1)
        let material = SimpleMaterial(color: .systemBlue, isMetallic: false)
        
        let entity2 = ModelEntity(mesh: mesh2, materials: [material])
        entity2.scale = SIMD3<Float>(0.01, 0.01, 0.01)
        entity2.setPosition(SIMD3<Float>(0, 0, 0.03), relativeTo: self)
        
        self.addChild(entity2)
    }
}
