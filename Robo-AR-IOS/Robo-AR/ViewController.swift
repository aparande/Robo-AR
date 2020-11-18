//
//  ViewController.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/2/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit
import RealityKit
import ARKit

enum Mode {
    case debug, release
}



class ViewController: BLEViewController {
    @IBOutlet var arView: WaypointView!
    @IBOutlet weak var generateButton: UIButton!
    
    let MODE = Mode.release
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        arView.addCoaching()
        arView.setupGestures()
        
        guard let referenceImages = ARReferenceImage.referenceImages(inGroupNamed: "AR Resources", bundle: nil) else {
            fatalError("Missing expected asset catalog resources.")
        }
        
        let config = ARWorldTrackingConfiguration()
        config.detectionImages = referenceImages
        config.planeDetection = .horizontal
        
        arView.session.delegate = self
        arView.session.run(config, options: [.resetTracking, .removeExistingAnchors])
        arView.debugOptions.insert(.showSceneUnderstanding)
        arView.debugOptions.insert(.showWorldOrigin)
        //arView.debugOptions.insert(.showAnchorGeometry)
        
        generateButton.layer.cornerRadius = 20
    }
    
    @IBAction func startTransmission(_ sender: Any) {
        if MODE == .debug {
            performSegue(withIdentifier: "showInstructions", sender: nil)
        } else {
            if instructions.isEmpty {
                instructions = compileInstructions()
            }
            sendNextInstruction()
        }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        guard let destination = segue.destination as? InstructionTableViewController else { return }
        destination.instructions = compileInstructions()
    }
    
    func compileInstructions() -> [Instruction] {
        var inst:[Instruction] = []
        
        var prevAngle: Float = 0.0
        for waypoint in arView.waypoints {
            guard let next = waypoint.next else { continue }
            inst.append(Instruction(distance: waypoint.distanceTo(next), angle: waypoint.angleTo(next) - prevAngle))
            prevAngle = waypoint.angleTo(next)
        }
        return inst
    }
    
    override func discoveredInstructionCharacteristic() {
        // Do Nothing
    }
}

extension ViewController: ARSessionDelegate {
    
    func session(_ session: ARSession, didAdd anchors: [ARAnchor]) {
        print("Entered Session Function")
        print(anchors.count)
        guard let objectAnchor = anchors.first as? ARImageAnchor,
              let _ = objectAnchor.referenceImage.name
        else { return }
        
        
        print("FOUND ANCHOR")
        let anchor = AnchorEntity(anchor: objectAnchor)
        print(anchor.transform.rotation)
        arView.scene.anchors.append(anchor)
       
    }
}
