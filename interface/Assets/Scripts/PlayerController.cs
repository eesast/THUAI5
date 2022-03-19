using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerController : MonoBehaviour
{
    private float _x;
    private float _y;
    private float _preX;
    private float _preY;

    private bool _isMoving;
    private bool _isInvisible;
    private bool _preIsInvisible;

    private Animator _animator;

    private GameObject _player;
    private GameObject _EnergyBall;
    public GameObject jamSignal;

    // Start is called before the first frame update
    void Start()
    {
        _player = transform.GetChild(0).gameObject;
        _EnergyBall = transform.GetChild(1).gameObject;

        _animator = _player.GetComponent<Animator>();
    }
    // Update is called once per frame
    void Update()
    {
        //更新_x,_y之前
        _preX = _x;
        _preY = _y;

        //更新_x,_y之后
        transform.position.Set(_x, _y, 0);

        //更新_isInvisible之前
        _preIsInvisible = _isInvisible;

        updateAnimation();
    }

    void updateAnimation()
    {
        if (Mathf.Approximately(_x, _preX) && Mathf.Approximately(_y, _preY))
        {
            _isMoving = false;
        }
        else
        {
            _isMoving = true;
        }

        _animator.SetBool("isMoving", _isMoving);

        _EnergyBall.transform.RotateAround(transform.position, Vector3.up, 150 * Time.deltaTime);
        _EnergyBall.transform.Rotate(0, -150 * Time.deltaTime, 0);
    }

    void GenerateJamSignal()
    {
        GameObject _jamSignal = Instantiate(jamSignal);
        _jamSignal.transform.SetParent(transform);
        _jamSignal.transform.localPosition = new Vector3(0, 4, 0);
    }
}
