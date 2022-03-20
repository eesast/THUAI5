using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class SceneTransfer : MonoBehaviour
{
    private float _timeCounter = 0;

    private void Update()
    {
        _timeCounter += Time.deltaTime;
        if (_timeCounter > 3)
        {
            SceneManager.LoadScene(1);
        }
    }
}
