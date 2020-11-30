
import UIKit
import RealityKit

class RoboWaypoint: ARPoint {
       
    var lastKnownWaypointDistance: Float?
    var lastKnownWaypointAngle: Float?
    var waypointLastKnownAngleTo: Float?
    var waypointLastKnownNumber: Int?
    var isTracking: Bool;
    
    required init(color: UIColor) {
        isTracking = false;
        super.init(color: color, text: "Robot")
        addSphere()
    }
    
    required init() {
        fatalError("init() has not been implemented")
    }
    
    override func angleTo(_ other: ARPoint) -> Float {
        
        var diff = other.parent!.position(relativeTo: self.parent!)
        diff.y = 0
        let forwardAngle = SIMD3<Float>(0.0, 0.0, 1.0)
        var angle = acos(dot(forwardAngle, diff) / length(diff)) * 180 / .pi
        if(cross(forwardAngle, diff).y  < 0){
            angle = -1 * angle
        }
        return angle
    }
        
    private func addSphere() {
        let mesh2 = MeshResource.generateSphere(radius: 1)
        let material = SimpleMaterial(color: .systemBlue, isMetallic: false)
        
        let entity2 = ModelEntity(mesh: mesh2, materials: [material])
        entity2.scale = SIMD3<Float>(0.01, 0.01, 0.01)
        entity2.setPosition(SIMD3<Float>(0, 0, 0.03), relativeTo: self)
        
        self.addChild(entity2)
    }
}
