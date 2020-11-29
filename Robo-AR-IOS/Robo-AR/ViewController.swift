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
import CoreBluetooth


class ViewController: BLEViewController {
    @IBOutlet var arView: WaypointView!
    @IBOutlet weak var generateButton: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        arView.addCoaching()
        arView.setupGestures()
        
        guard let referenceImages = ARReferenceImage.referenceImages(inGroupNamed: "AR Resources", bundle: nil) else {
            fatalError("Missing expected asset catalog resources.")
        }
        
        let config = ARWorldTrackingConfiguration()
        config.detectionImages = referenceImages
        config.maximumNumberOfTrackedImages = 1
        config.planeDetection = .horizontal
        
        arView.session.delegate = self
        arView.session.run(config, options: [.resetTracking, .removeExistingAnchors])
        arView.debugOptions.insert(.showSceneUnderstanding)
        arView.debugOptions.insert(.showWorldOrigin)
        
        generateButton.layer.cornerRadius = 20
    }
    
    @IBAction func startTransmission(_ sender: Any) {
        if let inst = compileNextInstruction() {
            sendInstruction(instruction: inst)
        }
    }
    
    override func compileNextInstruction() -> Instruction? {
        guard let robot = arView.robot else { return nil }
        
        if let currentWaypoint = arView.currentWayPoint {
            if(robot.distanceTo(currentWaypoint) < 0.10){
                arView.currentWayPoint = currentWaypoint.next
            }
        } else {
            arView.currentWayPoint = arView.waypoints.head
        }
        
        guard let waypoint = arView.currentWayPoint else { return nil }
        
        return Instruction(distance: robot.distanceTo(waypoint), angle: robot.angleTo(waypoint), waypointNumber: waypoint.number)
    }
}

extension ViewController: ARSessionDelegate {
    
    func session(_ session: ARSession, didAdd anchors: [ARAnchor]) {
        guard let imageAnchor = anchors.first as? ARImageAnchor,
              let _ = imageAnchor.referenceImage.name
        else { return }
        arView.addRobot(anchor: imageAnchor)
    }
    
    func session(_ session: ARSession, didUpdate anchors: [ARAnchor]) {
        guard let imageAnchor = anchors.first as? ARImageAnchor,
              let _ = imageAnchor.referenceImage.name
        else { return }
        arView.updateRobot(anchor: imageAnchor)
    }
}
