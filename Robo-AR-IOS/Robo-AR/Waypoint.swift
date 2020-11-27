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
            mesh: .generateBox(size: SIMD3<Float>(0.05, 0.05, 0.025)),
            materials: [SimpleMaterial(
                color: color,
                isMetallic: false)
            ]
        )
        
        constructText()
    }
    
    required init() {
        fatalError("init() has not been implemented")
    }
    
    
    func distanceTo(_ other:Waypoint) -> Float {
        return self.parent!.position(relativeTo: other.parent!).lengthHorizontal
    }
    
    func angleTo(_ other: Waypoint) -> Float {
        return self.parent!.position.horizontalAngle(to: other.parent!.position)
    }
    
    private func constructText() {
        let mesh = MeshResource.generateText(
            "\(self.number)",
            extrusionDepth: 0.1,
            font: .systemFont(ofSize: 2),
            containerFrame: .zero,
            alignment: .left,
            lineBreakMode: .byTruncatingTail)

        let material = SimpleMaterial(color: .systemBlue, isMetallic: false)
        let entity = ModelEntity(mesh: mesh, materials: [material])
        entity.scale = SIMD3<Float>(0.01, 0.01, 0.03)
        entity.setPosition(SIMD3<Float>(0.02, 0.04, 0), relativeTo: self)
        entity.setOrientation(simd_quatf(angle: -.pi/4, axis: [0, 0, 1]), relativeTo: self)
        
        self.addChild(entity)
    }
}
