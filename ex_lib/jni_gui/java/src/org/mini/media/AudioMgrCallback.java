/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.mini.media;

/**
 *
 * @author Gust
 */
public interface AudioMgrCallback {

    void onCapture(int millSecond, byte[] data);
    
    void onPlay(int millSecond, byte[] data);

    void onStop();
}
