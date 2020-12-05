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
        if let inst = compileNextInstruction(fromAck: false) {
            sendInstruction(instruction: inst)
        }
    }
    

    override func compileNextInstruction(fromAck: Bool) -> Instruction? {
        guard let robot = arView.robot else { return nil }
        
        print("ROBOT TRACKING", robot.isTracking)
        if(robot.isTracking){
            if let currentWaypoint = arView.currentWayPoint {
            
                if(robot.distanceTo(currentWaypoint) < 0.10){
                    arView.currentWayPoint = currentWaypoint.next
                    arView.waypoints.head = arView.currentWayPoint
                }
            } else {
                arView.currentWayPoint = arView.waypoints.head
            }
        
            guard let waypoint = arView.currentWayPoint else { return nil }
            
            return Instruction(distance: robot.distanceTo(waypoint), angle: robot.angleTo(waypoint), waypointNumber: waypoint.number)
        } else {
            
            guard let waypoint = arView.currentWayPoint else {return nil}
        
            if(!fromAck){
                return Instruction(distance: robot.lastKnownWaypointDistance!, angle: robot.lastKnownWaypointAngle!, waypointNumber: waypoint.number)
            }
            else {
                guard let nextWaypoint = waypoint.next else { return nil }
                
                
                var prevAngle: Float;
                if(waypoint.number == robot.waypointLastKnownNumber){
                    prevAngle = 180 - robot.waypointLastKnownAngleTo!
                } else{
                    guard let prevWaypoint = waypoint.prev else {return nil}
                    
                    prevAngle = 180 - waypoint.distanceTo(prevWaypoint)
                    print("PREVANGLE", prevAngle)
                    
                }
                
                let nextAngle = waypoint.angleTo(nextWaypoint)
                
                let finalAngle = nextAngle + prevAngle
                print("ANGLES!!!")
                print(prevAngle)
                print(nextAngle)
                print(finalAngle)
                arView.currentWayPoint = nextWaypoint
                
                return Instruction(distance: waypoint.distanceTo(nextWaypoint), angle: finalAngle, waypointNumber: nextWaypoint.number)
                
            }
            
            
        }
        
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
    
    func session(_ session: ARSession, didRemove anchors: [ARAnchor]) {
        guard let imageAnchor = anchors.first as? ARImageAnchor,
              let _ = imageAnchor.referenceImage.name
        else { return }
    }
}
