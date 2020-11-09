import UIKit
import RealityKit

class Waypoint: Entity, HasModel, HasAnchoring, HasCollision {
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
        super.init()
        
        self.components[ModelComponent] = ModelComponent(
            mesh: .generateBox(size: 0.1),
            materials: [SimpleMaterial(
                color: color,
                isMetallic: false)
            ]
        )
        
        constructText()
    }
    
    convenience init(color: UIColor, position: SIMD3<Float>, number: Int) {
        self.init(color: color, number: number)
        self.position = position
    }
    
    required init() {
        fatalError("init() has not been implemented")
    }
    
    private func constructText() {
        let mesh = MeshResource.generateText(
            "\(self.number)",
            extrusionDepth: 0.1,
            font: .systemFont(ofSize: 2),
            containerFrame: .zero,
            alignment: .left,
            lineBreakMode: .byTruncatingTail)

        let material = SimpleMaterial(color: .red, isMetallic: false)
        let entity = ModelEntity(mesh: mesh, materials: [material])
        entity.scale = SIMD3<Float>(0.03, 0.03, 0.1)
        entity.setPosition(SIMD3<Float>(0, 0.05, 0), relativeTo: self)
        
        self.addChild(entity)
    }
    
    private func distanceTo(_ other:Waypoint) -> Float {
        return other.position(relativeTo: self).magnitude
    }
}
